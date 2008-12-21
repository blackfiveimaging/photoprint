/*
 * layout_imageinfo.cpp - A base class for layout images.
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <string.h>
#include <gtk/gtk.h>

#include "config.h"

#include "dialogs.h"
#include "support/generaldialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "support/rotatepixbuf.h"
#include "support/maskpixbuf.h"
#include "support/thread.h"
#include "support/progressthread.h"

#include "imagesource/imagesource.h"
#include "imagesource/imagesource_gdkpixbuf.h"
#include "imagesource/imagesource_cms.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_mask.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_promote.h"
#include "imagesource/imagesource_invert.h"

#include "photoprint_state.h"
#include "support/progress.h"
#include "support/util.h"
#include "support/layoutrectangle.h"

#include "layout_imageinfo.h"

using namespace std;


Layout_ImageInfo::Layout_ImageInfo(Layout &layout, const char *filename, int page, bool allowcropping, PP_ROTATION rotation)
	: PPEffectHeader(), page(page), allowcropping(allowcropping), crop_hpan(CENTRE), crop_vpan(CENTRE),
	rotation(rotation), layout(layout), maskfilename(NULL), thumbnail(NULL), mask(NULL), hrpreview(NULL),
	selected(false), customprofile(NULL), customintent(LCMSWRAPPER_INTENT_DEFAULT), hrrenderthread(NULL)
{
	bool relative=true;

	if(filename[0]=='/' || filename[1]==':')
		relative=false;

	if(filename[0]=='\\' && filename[1]=='\\')
		relative=false;

	if(relative)
		this->filename=BuildAbsoluteFilename(filename);
	else
		this->filename=strdup(filename);

	ImageSource *is=ISLoadImage(this->filename);
	width=is->width;
	height=is->height;
	xres=is->xres;
	yres=is->yres;
	// FIXME - can we grab the embedded profile's name here (for the ImageInfo widget)?
	delete is;

	GetThumbnail();
}


Layout_ImageInfo::Layout_ImageInfo(Layout &layout, Layout_ImageInfo *ii, int page, bool allowcropping, PP_ROTATION rotation)
	: PPEffectHeader(*ii), page(page), allowcropping(allowcropping), crop_hpan(CENTRE), crop_vpan(CENTRE),
	rotation(rotation), layout(layout), maskfilename(NULL), thumbnail(NULL), mask(NULL), hrpreview(NULL),
	selected(false), customprofile(NULL), customintent(LCMSWRAPPER_INTENT_DEFAULT), hrrenderthread(NULL)
{
	// Effects are copied by the "PPEffectHeader(*ii) above
	if(ii)
	{
		// We duplicate and reference the thumbnail and mask from the old image...
		thumbnail=ii->thumbnail;
		if(thumbnail)
			g_object_ref(G_OBJECT(thumbnail));
		mask=ii->mask;
		if(mask)
			g_object_ref(G_OBJECT(mask));

		// Duplicate name of manually-applied profile...
		if(ii->customprofile)
		{
			customprofile=strdup(ii->customprofile);
			cerr << "Copying profile: " << ii->customprofile << endl;
		}
		customintent=ii->customintent;
	
		// Copy general data
		if(ii->filename)
			this->filename=strdup(ii->filename);
		if(ii->maskfilename)
			this->maskfilename=strdup(ii->maskfilename);

		allowcropping=ii->allowcropping;
		crop_hpan=ii->crop_hpan;
		crop_vpan=ii->crop_vpan;
		rotation=ii->rotation;
		width=ii->width;
		height=ii->height;
		xres=ii->xres;
		yres=ii->yres;
	}
}


Layout_ImageInfo::~Layout_ImageInfo()
{
	// Must flush the HRPreview and any rendering thread first - this ensures the thread isn't running any more.
	FlushHRPreview();
	ObtainMutex();
	if(thumbnail)
		g_object_unref(thumbnail);
	if(mask)
		g_object_unref(mask);
	layout.imagelist=g_list_remove(layout.imagelist,this);
	if(customprofile)
		free(customprofile);
	free(filename);
	cerr << "Layout_ImageInfo successfully disposed" << endl;
}


// Subthread for rendering high-resolution previews.
// This code uses the subthread to create a GdkPixbuf from
// an image, then defers to the main thread to perform the
// actual rendering.

class hr_payload
{
	public:
	hr_payload(ProfileManager *p,CMTransformFactory *f,Layout_ImageInfo *ii,GtkWidget *wid,int x,int y,int w,int h)
		: profman(p), factory(f), ii(ii), widget(wid), xpos(x), ypos(y), width(w), height(h), transformed(NULL), thread(NULL)
	{
	}
	~hr_payload()
	{
		if(thread)
			delete thread;
	}
	static int ThreadFunc(Thread *t,void *ud)
	{
		hr_payload *p=(hr_payload *)ud;

		p->thread=t;

		cerr << "Subthread - about to obtain mutex" << endl;
		p->ii->ObtainMutexShared();

		t->SendSync();

		// We sleep briefly before doing anything time-intensive - that way we can bail out rapidly
		// if the user's doing something heavily interactive, like panning an image, or resizing the window

		for(int i=0;i<75;++i)
		{
#ifdef WIN32
			Sleep(10);
#else
			usleep(10000);
#endif
			if(t->TestBreak())
			{
				cerr << "Got break signal while pausing - Releasing" << endl;
				p->ii->ReleaseMutex();
				g_timeout_add(1,hr_payload::CleanupFunc,p);
				return(0);
			}
		}

		if(t->TestBreak())
		{
			cerr << "Subthread releasing mutex and cancelling" << endl;
			p->ii->ReleaseMutex();
			g_timeout_add(1,hr_payload::CleanupFunc,p);
			return(0);
		}

		try
		{
			CMSProfile *targetprof;

			CMColourDevice tdev=CM_COLOURDEVICE_NONE;
			if((targetprof=p->profman->GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
				tdev=CM_COLOURDEVICE_PRINTERPROOF;
			else if((targetprof=p->profman->GetProfile(CM_COLOURDEVICE_DISPLAY)))
				tdev=CM_COLOURDEVICE_DISPLAY;
			else if((targetprof=p->profman->GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
				tdev=CM_COLOURDEVICE_DEFAULTRGB;
			if(targetprof)
				delete targetprof;

			if(t->TestBreak())
			{
				cerr << "Subthread releasing mutex and cancelling" << endl;
				p->ii->ReleaseMutex();
				g_timeout_add(1,hr_payload::CleanupFunc,p);
				return(0);
			}

			cerr << "Generating high-res preview - Using tdev: " << tdev << endl;

			ImageSource *is=p->ii->GetImageSource(tdev,p->factory);

			cerr << "Got imagesource - fitting and rendering" << endl;

			LayoutRectangle r(is->width,is->height);
			LayoutRectangle target(p->xpos,p->ypos,p->width,p->height);

			RectFit *fit=r.Fit(target,p->ii->allowcropping,p->ii->rotation,p->ii->crop_hpan,p->ii->crop_vpan);

			if(fit->rotation)
				is=new ImageSource_Rotate(is,fit->rotation);
			is=ISScaleImageBySize(is,fit->width,fit->height,IS_SCALING_AUTOMATIC);
			delete fit;
			// We create new Fit in the idle-function because the hpan/vpan may have changed.

			ProgressThread prog(*t);
			p->transformed=pixbuf_from_imagesource(is,p->ii->layout.bgcol.red>>8,p->ii->layout.bgcol.green>>8,p->ii->layout.bgcol.blue>>8,&prog);

			delete is;

			cerr << "finished - finalising" << endl;

			if(p->transformed)
			{
				// Now we defer to the main thread...
				// We add this as a high-priority event because we want it to be
				// run as soon as possible, and within a gtk_main_iteration() loop
				// if necessary.
	//			g_idle_add_full(G_PRIORITY_HIGH,hr_payload::IdleFunc,p,NULL);
				g_timeout_add(1,hr_payload::IdleFunc,p);

				// And wait for the main thread to have rendered the preview
				// Because the rendering will be done via a GTK event callback, there's
				// a potential deadlock here if the main app attempts to delete this ImageInfo
				// between the main thread having completed and the idle function being
				// triggered to draw the thumbnail.  For this reason we'll have to use a
				// tie-break in the ImageInfo destructor.
	//			cerr << "Waiting for all-clear from main thread..." << endl;
	//			t->WaitSync();
			}
			else
			{
				cerr << "Thread cancelled" << endl;
				g_timeout_add(1,hr_payload::CleanupFunc,p);
				p->ii->ReleaseMutex();
				return(0);
			}
		}
		catch (const char *err)
		{
			cerr << "Subthread caught exception: " << err << endl;
			g_timeout_add(1,hr_payload::CleanupFunc,p);
		}
		cerr << "Subthread waiting for main thread to finish drawing" << endl;
		p->thread->WaitSync();
		cerr << "Subthread releasing mutex and exiting" << endl;
		p->ii->ReleaseMutex();
		return(0);
	}
	static gboolean CleanupFunc(gpointer ud)
	{
		hr_payload *p=(hr_payload *)ud;
		p->thread->SendSync();
		delete p;
		return(FALSE);
	}
	static gboolean IdleFunc(gpointer ud)
	{
		// Once control reaches here the subthread should have
		// completed.  There's a brief window in which the ImageInfo
		// could have been deleted by the main thread before this idle-handler
		// was launched. To fix this, we hold the mutex in the sub-thread, until
		// this function, running in the main context, has finished with the
		// ImageInfo.

		hr_payload *p=(hr_payload *)ud;

		// This function runs in the context of the main thread, so it's safe
		// to clear the ImageInfo's RenderThread member, since only the main thread
		// creates such.
		// This idle-function is responsible for disposing of the payload, which
		// will also delete the thread.

		if(!p->thread->TestBreak())
		{
			if(p->ii->hrrenderthread==p->thread)
				p->ii->hrrenderthread=NULL;

			p->ii->SetHRPreview(p->transformed);

			LayoutRectangle r(gdk_pixbuf_get_width(p->transformed),gdk_pixbuf_get_height(p->transformed));
			LayoutRectangle target(p->xpos,p->ypos,p->width,p->height);

			// Disallow rotation here since the image will be rotated already.
			RectFit *fit=r.Fit(target,p->ii->allowcropping,PP_ROTATION_NONE,p->ii->crop_hpan,p->ii->crop_vpan);
			int dw=fit->width;
			int dh=fit->height;
		
			if(dw > p->width)
				dw=p->width;

			if(dh > p->height)
				dh=p->height;
			
			if(dw>gdk_pixbuf_get_width(p->transformed))
			{
				cerr << "DW too high" << endl;
				dw=gdk_pixbuf_get_width(p->transformed);
			}

			if(dh>gdk_pixbuf_get_height(p->transformed))
			{
				cerr << "DH too high" << endl;
				dh=gdk_pixbuf_get_height(p->transformed);
			}

			if(p->ii->mask)
			{
				p->transformed=gdk_pixbuf_copy(p->transformed);
				maskpixbuf(p->transformed,fit->xoffset,fit->yoffset,dw,dh,p->ii->mask,
					p->ii->layout.bgcol.red>>8,p->ii->layout.bgcol.green>>8,p->ii->layout.bgcol.blue>>8);
			}

			gdk_draw_pixbuf(p->widget->window,NULL,p->transformed,
				fit->xoffset,fit->yoffset,
				fit->xpos,fit->ypos,
				dw,dh,
				GDK_RGB_DITHER_NONE,0,0);

			if(p->ii->mask)
				g_object_unref(p->transformed);

			delete fit;
		}
		cerr << "Preview drawn - sending sync to sub-thread" << endl;
		p->thread->SendSync();

		delete p;

		return(FALSE);
	}
	protected:
	ProfileManager *profman;
	CMTransformFactory *factory;
	Layout_ImageInfo *ii;
	GtkWidget *widget;
	int xpos,ypos;
	int width,height;
	GdkPixbuf *transformed;
	Thread *thread;
};


void Layout_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	GdkPixbuf *thumbnail=hrpreview;
	GdkPixbuf *transformed=NULL;
	RectFit *fit=NULL;
	int dw,dh;

	// Since the thread (or rather, the idle function it triggers)
	// is now responsible for its own demise, we no longer have to wait for it.
	// Furthermore, if there's a thread running, we needn't cancel it here - instead
	// we can allow it to continue.  Only changes which would lead to flushing the
	// hi-res preview need to cancel the thread...
//	cerr << "Cancelling previous thread" << endl;
//	if(hrrenderthread)
//		hrrenderthread->Stop();
//	else
//		cerr << "No thread" << endl;
#if 0
	if(hrrenderthread)
	{
		hrrenderthread->Stop();
		while(!hrrenderthread->TestFinished())
			gtk_main_iteration();
		delete hrrenderthread;
	}
#endif
//	hrrenderthread=NULL;

	cerr << "DrawThumbnail - Obtain" << endl;
	if(thumbnail)
	{
		LayoutRectangle r(gdk_pixbuf_get_width(thumbnail),gdk_pixbuf_get_height(thumbnail));
		LayoutRectangle target(xpos,ypos,width,height);

		fit=r.Fit(target,allowcropping,PP_ROTATION_NONE,crop_hpan,crop_vpan);

		dw=fit->width;
		dh=fit->height;
		
		if(dw > width)
			dw=width;

		if(dh > height)
			dh=height;
		
		if(dw>gdk_pixbuf_get_width(thumbnail))
		{
			cerr << "DW too high" << endl;
			dw=gdk_pixbuf_get_width(thumbnail);
		}

		if(dh>gdk_pixbuf_get_height(thumbnail))
		{
			cerr << "DH too high" << endl;
			dh=gdk_pixbuf_get_height(thumbnail);
		}
		if(mask)
		{
			transformed=gdk_pixbuf_copy(hrpreview);
			maskpixbuf(transformed,fit->xoffset,fit->yoffset,dw,dh,mask,
				layout.bgcol.red>>8,layout.bgcol.green>>8,layout.bgcol.blue>>8);
			thumbnail=transformed;
		}
	}
	else
	{
		thumbnail=GetThumbnail();
		
		LayoutRectangle r(gdk_pixbuf_get_width(thumbnail),gdk_pixbuf_get_height(thumbnail));
		LayoutRectangle target(xpos,ypos,width,height);

		fit=r.Fit(target,allowcropping,rotation,crop_hpan,crop_vpan);

		GdkPixbuf *tmp;
		switch(fit->rotation)
		{
			case 0:
				transformed=gdk_pixbuf_scale_simple(thumbnail,fit->width,fit->height,GDK_INTERP_NEAREST);
				break;
			case 270:
				tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_CLOCKWISE);
				transformed=gdk_pixbuf_scale_simple(tmp,fit->width,fit->height,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
				break;
			case 180:
				tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_UPSIDEDOWN);
				transformed=gdk_pixbuf_scale_simple(tmp,fit->width,fit->height,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
				break;
			case 90:
				tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
				transformed=gdk_pixbuf_scale_simple(tmp,fit->width,fit->height,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
				break;
		}

		dw=fit->width;
		dh=fit->height;
		
		if(dw > width)
			dw=width;

		if(dh > height)
			dh=height;
		
		if(dw>gdk_pixbuf_get_width(transformed))
		{
			cerr << "DW too high" << endl;
			dw=gdk_pixbuf_get_width(transformed);
		}

		if(dh>gdk_pixbuf_get_height(transformed))
		{
			cerr << "DH too high" << endl;
			dh=gdk_pixbuf_get_height(transformed);
		}

		if(mask)
			maskpixbuf(transformed,fit->xoffset,fit->yoffset,dw,dh,mask,
				layout.bgcol.red>>8,layout.bgcol.green>>8,layout.bgcol.blue>>8);
		thumbnail=transformed;

		// Trigger a rendering thread if there isn't one already
		// and if high-res previews are enabled
		if(hrrenderthread==NULL && layout.state.FindInt("HighresPreviews"))
		{
			cerr << "Launching render thread" << endl;
			if(width>192 && height>192) // Generating lots of thumbs simultaneously is expensive, and if the images are small,
			{							// highres previews are of limited value anyway.
				hr_payload *p=new hr_payload(&layout.state.profilemanager,layout.factory,this,widget,xpos,ypos,width,height);
				hrrenderthread=new Thread(hr_payload::ThreadFunc,p);
				hrrenderthread->Start();
				hrrenderthread->WaitSync();
			}
		}
	}
	cerr << "ObtainThumbnail - Release" << endl;


	gdk_draw_pixbuf(widget->window,NULL,thumbnail,
		fit->xoffset,fit->yoffset,
		fit->xpos,fit->ypos,
		dw,dh,
		GDK_RGB_DITHER_NONE,0,0);

	if(transformed)
		g_object_unref(transformed);

//	cerr << "Waiting for sync" << endl;
//	if(hrrenderthread)
//		hrrenderthread->WaitSync();
//	else
//		cerr << "No thread" << endl;
//	cerr << "Done" << endl;

	delete fit;
}


void Layout_ImageInfo::SetMask(const char *filename)
{
	if(mask)
	{
		g_object_unref(mask);
		mask=NULL;
	}
	if(maskfilename)
	{
		free(maskfilename);
		maskfilename=NULL;
	}
	if(filename)
		maskfilename=strdup(filename);
	FlushThumbnail();
}


GdkPixbuf *Layout_ImageInfo::GetThumbnail()
{
	if(thumbnail)
		return(thumbnail);

	GError *err=NULL;

	if(layout.state.batchmode)
		return(NULL);

	cerr << "Getting thumbnail" << endl;

	if(maskfilename && !mask)
	{
		mask=egg_pixbuf_get_thumbnail_for_file (maskfilename, EGG_PIXBUF_THUMBNAIL_LARGE, &err);
//		cerr << "Attempting to load mask from: " << maskfilename << endl;
		if(!mask)
		{
//			cerr << "Failed." << endl;
			try
			{
				ImageSource *src=ISLoadImage(maskfilename);
				if(src)
				{
					int w,h;
					w=(src->width*256)/src->height;
					h=256;
					if(w>256)
					{
						w=256;
						h=(src->height*256)/src->width;
					}
					src=ISScaleImageBySize(src,w,h,IS_SCALING_NEARESTNEIGHBOUR);
					mask=pixbuf_from_imagesource(src);
					delete src;
				}
			}
			catch(const char *err)
			{
				cerr << "Error: " << err << endl;
			}	
			if(!mask)
			{
				if(err && err->message)
					cerr << "Error: " << err->message << endl;
				else
					cerr << "Can't get mask thumbnail" << endl;
				free(maskfilename);
				maskfilename=NULL;
			}
		}
	}

//	if(thumbnail)
//		return(thumbnail);

	cerr << "Thumbnail not cached - loading..." << endl;

	ImageSource *src=NULL;
		
	thumbnail=egg_pixbuf_get_thumbnail_for_file (filename, EGG_PIXBUF_THUMBNAIL_LARGE, &err);

	if(!thumbnail)
	{
		cerr << "Can't get pixbuf - loading thumbnail via ImageSource..." << endl;
		src=ISLoadImage(filename);
		if(src)
		{
			int w,h;
			w=(src->width*256)/src->height;
			h=256;
			if(w>256)
			{
				w=256;
				h=(src->height*256)/src->width;
			}
			src=ISScaleImageBySize(src,w,h,IS_SCALING_NEARESTNEIGHBOUR);
			thumbnail=pixbuf_from_imagesource(src);
			delete src;
			src=NULL;
		}

		if(!thumbnail)
		{
			if(err && err->message)
				throw err->message;
			else
				throw "Can't get thumbnail";
		}
	}

	// Apply effects here...
	if(thumbnail)
	{
		ImageSource *src2=new ImageSource_GdkPixbuf(thumbnail);
		src2=ApplyEffects(src2,PPEFFECT_PRESCALE);
		GdkPixbuf *tn2=pixbuf_from_imagesource(src2);
		delete src2;
		g_object_unref(G_OBJECT(thumbnail));
		thumbnail=tn2;
	}


	// If there's no display profile, then we can use the Default RGB profile instead...
//	cerr << "Checking for Display Profile..." << endl;
	CMSProfile *targetprof;
	CMColourDevice target=CM_COLOURDEVICE_NONE;
	if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
//		target=CM_COLOURDEVICE_DISPLAY;
		target=CM_COLOURDEVICE_PRINTERPROOF;
	else if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
		target=CM_COLOURDEVICE_DISPLAY;
	else if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
		target=CM_COLOURDEVICE_DEFAULTRGB;
	if(targetprof)
		delete targetprof;

	if(target!=CM_COLOURDEVICE_NONE)
	{
		cerr << "Found - loading image to check for embedded profile..." << endl;
		if(!src)
		{
			src=ISLoadImage(filename);
		}

		CMSTransform *transform=NULL;
		if(src)
		{
			CMSProfile *emb;
			if(customprofile)
				emb=new CMSProfile(customprofile);  // FIXME: lifespan?
			else
				emb=src->GetEmbeddedProfile();
			if(emb)
			{
//				cerr << "Creating embedded->monitor transform..." << endl;
				if(emb->GetColourSpace()!=IS_TYPE_RGB)
				{
//					Need to replace the RGB thumbnail with a CMYK or Greyscale version!
					cerr << "Creating new thumbnail - CMYK->monitor" << endl;
					int w,h;
					w=(src->width*256)/src->height;
					h=256;
					if(w>256)
					{
						w=256;
						h=(src->height*256)/src->width;
					}
					src=ISScaleImageBySize(src,w,h,IS_SCALING_NEARESTNEIGHBOUR);
					if((transform = layout.factory->GetTransform(target,emb,customintent)))
						src=new ImageSource_CMS(src,transform);
					thumbnail=pixbuf_from_imagesource(src);
					delete src;
					src=NULL;
					transform=NULL; // Don't want to apply the transform a second time...
				}
				else
				{
					transform = layout.factory->GetTransform(target,emb,customintent);
				}
			}
			else
			{
//				cerr << "Creating default->monitor transform..." << endl;
				transform = layout.factory->GetTransform(target,IS_TYPE_RGB,customintent);
			}
		}
		if(transform)
		{
//			cerr << "Applying transform..." << endl;
			ImageSource *src2=new ImageSource_GdkPixbuf(thumbnail);
			src2=new ImageSource_CMS(src2,transform);
			GdkPixbuf *tn2=pixbuf_from_imagesource(src2);
			delete src2;
			g_object_unref(G_OBJECT(thumbnail));
			thumbnail=tn2;
		}
	}

	if(src)
	{
		delete src;		
	}

	cerr << "done" << endl;

	return(thumbnail);
}


LayoutRectangle *Layout_ImageInfo::GetBounds()
{
	// Dummy function - override in subclasses!
//	LayoutRectangle *result=new LayoutRectangle(0,0,100,100);
//	return(result);
	throw "Layout_ImageInfo::GetBounds() method should be overridden by subclass!";
}


RectFit *Layout_ImageInfo::GetFit(double scale)
{
	RectFit *result=NULL;
	LayoutRectangle *bounds=GetBounds();
	bounds->Scale(scale);

	LayoutRectangle r(width,height);
	result=r.Fit(*bounds,allowcropping,rotation,crop_hpan,crop_vpan);

	delete bounds;
	return(result);
}


ImageSource *Layout_ImageInfo::GetImageSource(CMColourDevice target,CMTransformFactory *factory)
{
	ImageSource *result=NULL;
	ImageSource *is=ISLoadImage(filename);

	is=ApplyEffects(is,PPEFFECT_PRESCALE);

	IS_TYPE colourspace=layout.GetColourSpace(target);

	if(STRIP_ALPHA(is->type)==IS_TYPE_GREY)
		is=new ImageSource_Promote(is,colourspace);

	if(STRIP_ALPHA(is->type)==IS_TYPE_BW)
		is=new ImageSource_Promote(is,colourspace);

	CMSTransform *transform=NULL;

	if(factory)
	{
		CMSProfile *emb;
		if(customprofile)
			emb=new CMSProfile(customprofile);  // FIXME: Lifespan!
		else
			emb=is->GetEmbeddedProfile();
		
		if(emb)
		{
//				cerr << "Has embedded / assigned profile..." << endl;
			transform=factory->GetTransform(target,emb,customintent);  // FIXME: intent!
		}
		else
		{
//				cerr << "No embedded profile - using default" << endl;
			transform=factory->GetTransform(target,IS_TYPE(STRIP_ALPHA(is->type)),customintent);
		}

		if(transform)
			is=new ImageSource_CMS(is,transform);
	}
	result=is;
	return(result);
}


ImageSource *Layout_ImageInfo::ApplyMask(ImageSource *is)
{
	if(maskfilename)
	{
		ImageSource *mask=ISLoadImage(maskfilename);
		if((is->width>is->height)^(mask->width>mask->height))
		{
			mask=new ImageSource_Rotate(mask,90);
//			cerr << "Rotating mask" << endl;
		}
		mask=ISScaleImageBySize(mask,is->width,is->height,IS_SCALING_AUTOMATIC);
		mask=new ImageSource_Invert(mask);
		is=new ImageSource_Mask(is,mask);
	}
	return(is);
}


bool Layout_ImageInfo::GetSelected()
{
	return(selected);
}


void Layout_ImageInfo::SetSelected(bool sel)
{
	selected=sel;
}


void Layout_ImageInfo::ToggleSelected()
{
	selected=!selected;
}


const char *Layout_ImageInfo::GetFilename()
{
	return(filename);
}


void Layout_ImageInfo::FlushThumbnail()
{
	if(thumbnail)
		g_object_unref(thumbnail);
	thumbnail=NULL;
	FlushHRPreview();
}


void Layout_ImageInfo::CancelRenderThread()
{
	if(hrrenderthread)
	{
		hrrenderthread->Stop();
//		while(!hrrenderthread->TestFinished())
//			gtk_main_iteration();
//		delete hrrenderthread;
	}
	hrrenderthread=NULL;
}


void Layout_ImageInfo::FlushHRPreview()
{
	CancelRenderThread();
	if(hrpreview)
		g_object_unref(hrpreview);
	hrpreview=NULL;
}


void Layout_ImageInfo::SetHRPreview(GdkPixbuf *preview)
{
	if(hrpreview)
		g_object_unref(hrpreview);
	hrpreview=NULL;
	hrpreview=preview;
}


void Layout_ImageInfo::AssignProfile(const char *filename)
{
	if(customprofile)
		free(customprofile);
	customprofile=NULL;
	if(filename)
		customprofile=strdup(filename);
//	if(customprofile)
//		cerr << "AssignProfile:  Custom Profile now set to " << customprofile << endl;
//	else
//		cerr << "AssignProfile:  No custom profile set" << endl;
	FlushThumbnail();
}


const char *Layout_ImageInfo::GetAssignedProfile()
{
	return(customprofile);
}


void Layout_ImageInfo::SetRenderingIntent(LCMSWrapper_Intent intent)
{
	FlushThumbnail();
	customintent=intent;
}


LCMSWrapper_Intent Layout_ImageInfo::GetRenderingIntent()
{
	return(customintent);
}


int Layout_ImageInfo::GetWidth()
{
	return(width);
}


int Layout_ImageInfo::GetHeight()
{
	return(height);
}


int Layout_ImageInfo::GetXRes()
{
	return(xres);
}


int Layout_ImageInfo::GetYRes()
{
	return(yres);
}


void Layout_ImageInfo::ObtainMutex()
{
	cerr << "In custom Obtain method - flushing preview..." << endl;
	FlushHRPreview();
	cerr << "Now attempting to obtain exclusive lock..." << endl;
	while(!PPEffectHeader::AttemptMutex())
	{
		cerr << "Can't get exclusive lock - performing main loop iteration" << endl;
		gtk_main_iteration();
	}
	cerr << "Done" << endl;
}

