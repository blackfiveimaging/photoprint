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
#include "miscwidgets/generaldialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imageutils/rotatepixbuf.h"
#include "imageutils/maskpixbuf.h"
#include "support/thread.h"
#include "support/progressthread.h"
#include "miscwidgets/errordialogqueue.h"
#include "imagesource/imagesource.h"
#include "imagesource/imagesource_gdkpixbuf.h"
#include "imagesource/imagesource_cms.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_mask.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_promote.h"
#include "imagesource/imagesource_invert.h"

#include "imageutils/cachedimage.h"
#include "imageutils/tiffsave.h"

#include "photoprint_state.h"

#include "support/debug.h"

#include "support/progress.h"
#include "support/util.h"
#include "support/layoutrectangle.h"

#include "layout_imageinfo.h"

using namespace std;


class PPIS_Histogram : public ImageSource
{
	public:
	PPIS_Histogram(ImageSource *source,PPHistogram &hist) : ImageSource(source), lock(hist), source(source,hist), histogram(hist)
	{
		Debug[TRACE] << "PPIS_Histogram obtaining Histogram mutex in exclusive mode from " << long(Thread::GetThreadID()) << endl;
		Debug[TRACE] << "Histogram address: " << long(&histogram) << endl;
	}
	virtual ~PPIS_Histogram()
	{
		Debug[TRACE] << "PPIS_Histogram triggering complete signal" << endl;
		histogram.Trigger();
		Debug[TRACE] << "PPIS_Histogram releasing Histogram mutex from " << long(Thread::GetThreadID()) << endl;
	}
	virtual ISDataType *GetRow(int row)
	{
		return(source.GetRow(row));
	}
	protected:
	PTMutex::Lock lock;
	ImageSource_Histogram source;
	PPHistogram &histogram;
};


Layout_ImageInfo::Layout_ImageInfo(Layout &layout, const char *filename, int page, bool allowcropping, PP_ROTATION rotation)
	: PPEffectHeader(), RefCountUI(), page(page), allowcropping(allowcropping), crop_hpan(CENTRE), crop_vpan(CENTRE),
	rotation(rotation), layout(layout), maskfilename(NULL), thumbnail(NULL), mask(NULL), hrpreview(NULL),
	selected(false), customprofile(NULL), customintent(LCMSWRAPPER_INTENT_DEFAULT),
	threadevents(), histogram(threadevents), hrrenderjob(NULL)
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
	if(!is)
		throw "Can't open image!";
	width=is->width;
	height=is->height;
	xres=is->xres;
	yres=is->yres;
	// FIXME - can we grab the embedded profile's name here (for the ImageInfo widget)?
	delete is;

	GetThumbnail();
}


Layout_ImageInfo::Layout_ImageInfo(Layout &layout, Layout_ImageInfo *ii, int page)
	: PPEffectHeader(*ii), RefCountUI(), page(page), allowcropping(false), crop_hpan(CENTRE), crop_vpan(CENTRE),
	rotation(PP_ROTATION_AUTO), layout(layout), maskfilename(NULL), thumbnail(NULL), mask(NULL), hrpreview(NULL),
	selected(false), customprofile(NULL), customintent(LCMSWRAPPER_INTENT_DEFAULT),
	threadevents(), histogram(threadevents), hrrenderjob(NULL)
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
			Debug[TRACE] << "Copying profile: " << ii->customprofile << endl;
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
	Debug[COMMENT] << "In Layout_ImageInfo destructor for " << long(this) << endl;
	FlushHRPreview();
	layout.Remove(this);
	ObtainMutex();
	if(thumbnail)
		g_object_unref(thumbnail);
	if(mask)
		g_object_unref(mask);

	if(customprofile)
		free(customprofile);
	free(filename);
	Debug[COMMENT] << "Layout_ImageInfo successfully disposed" << endl;
}


// Jobqueue-based replacement for the previous high-res preview code.
// This should be cleaner, and should take care of some of the concurrency issues behind the scenes.

class HRRenderJob : public Job, public Progress
{
	public:
	HRRenderJob(Layout_ImageInfo *ii,GtkWidget *wid,int x,int y,int w,int h)
		: Job(), Progress(), ii(ii), widget(wid), xpos(x), ypos(y), width(w), height(h), transformed(NULL), sync()
	{
		// Need to ref the ImageInfo here.
		Debug[TRACE] << "Creating HRRenderJob " << long(this) << endl;
		ii->Ref();
	}
	virtual ~HRRenderJob()
	{
		Debug[TRACE] << "Deleting HRRenderJob " << long(this) << " - unreferencing ImageInfo..." << endl;
		ii->UnRef();
		Debug[TRACE] << "Done - HRRenderJob disposed" << endl;
	}
	bool DoProgress(int i, int maxi)
	{
		return(GetJobStatus()!=JOBSTATUS_CANCELLED);
	}
	static bool testbreak(void *p)
	{
		HRRenderJob *j=(HRRenderJob *)p;
		return(j->DoProgress(0,0)==false);
	}
	virtual void Run(Worker *w)
	{
		ImageInfo_Worker *iw=(ImageInfo_Worker *)w;
		ImageSource *is=NULL;

		RWMutex::SharedLock lock(*ii,false);

		try
		{
			// Lock the imageinfo against modification while we're using it.
			// To avoid a deadlock situation if, say, the Apply Profile dialog is open and GTK decides that
			// now is a good time to redraw the window, we only attempt the mutex, and bail out if it fails.
			int count=10;
			while(!lock.Attempt())
			{
				if(count==0)
				{
					Debug[TRACE] << "HRRenderJob: Giving up attempt on mutex - bailing out" << endl;
					return;
				}
#ifdef WIN32
				Sleep(50);
#else
				usleep(50000);
#endif
				--count;
			}


			// We sleep briefly before doing anything time-intensive - that way we can bail out rapidly
			// if the user's doing something heavily interactive, like panning an image, or resizing the window

			for(int i=0;i<25;++i)
			{
#ifdef WIN32
				Sleep(10);
#else
				usleep(10000);
#endif
				if(!DoProgress(0,0))
				{
					Debug[TRACE] << "Got break signal while pausing - Releasing" << endl;
					return;
				}
			}


			// Now we start the rendering.

			CMSProfile *targetprof;

			CMColourDevice tdev=CM_COLOURDEVICE_NONE;
			if((targetprof=iw->profilemanager.GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
				tdev=CM_COLOURDEVICE_PRINTERPROOF;
			else if((targetprof=iw->profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
				tdev=CM_COLOURDEVICE_DISPLAY;
			else if((targetprof=iw->profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
				tdev=CM_COLOURDEVICE_DEFAULTRGB;
			if(targetprof)
				delete targetprof;

			// We declare is outside the try block so we can clean up if it fails.
			is=ii->GetImageSource(tdev,iw->factory);

			LayoutRectangle r(is->width,is->height);
			LayoutRectangle target(xpos,ypos,width,height);

			RectFit *fit=r.Fit(target,ii->allowcropping,ii->rotation,ii->crop_hpan,ii->crop_vpan);

			if(fit->rotation)
			{
				ImageSource_Interruptible *ii=new ImageSource_Rotate(is,fit->rotation);
				ii->SetTestBreak(testbreak,this);
				is=ii;
			}

			is=ISScaleImageBySize(is,fit->width,fit->height,IS_SCALING_AUTOMATIC);
			delete fit;
			// We create new Fit in the idle-function because the hpan/vpan may have changed.

			// Instead of building the GdkPixbuf here we create a cached image and convert to pixbuf in the main thread.
			transformed=new CachedImage(RefCountPtr<ImageSource>(is));

			Debug[TRACE] << "Built cached image -rendering" << endl;

			if(transformed)
			{
				Debug[TRACE] << "Calling DoProgress" << endl;
				// Now we defer to the main thread...
 				if(DoProgress(0,0))
				{
					Debug[TRACE] << "Calling finish_main" << endl;
					g_timeout_add(1,finish_main,this);
					Debug[TRACE] << "Waiting for sync from subthread" << endl;
					sync.WaitCondition();
					Debug[TRACE] << "Received sync from main thread - Job complete, deleting transformed..." << endl;
				}
				delete transformed;
			}
			else
				Debug[TRACE] << "RenderHRJob cancelled - detected from subthread" << endl;
		}
		catch(const char *err)
		{
			if(is)
			{
				Debug[TRACE] << "Deleting ImageSource chain..." << endl;
				delete is;
			}

			Debug[TRACE] << "HRRenderJob Caught error: " << err << endl;
			ErrorDialogs.AddMessage(err);
		}
	}

	// IdleFunc - runs on the main thread's context,
	// thus, can safely render into the UI.
	static gboolean finish_main(gpointer ud)
	{
		Debug[TRACE] << "In finish_main function..." << endl;
		HRRenderJob *p=(HRRenderJob *)ud;
		try
		{
			if(p->DoProgress(0,0))
			{
				Debug[TRACE] << "Creating pixbuf from CachedImage" << endl;
				ImageSource *is=new ImageSource_CachedImage(p->transformed);

				GdkPixbuf *transformed=pixbuf_from_imagesource(is,p->ii->layout.bgcol.red>>8,p->ii->layout.bgcol.green>>8,p->ii->layout.bgcol.blue>>8,p);

				LayoutRectangle r(gdk_pixbuf_get_width(transformed),gdk_pixbuf_get_height(transformed));
				LayoutRectangle target(p->xpos,p->ypos,p->width,p->height);

				// Disallow rotation here since the image will be rotated already.
				RectFit *fit=r.Fit(target,p->ii->allowcropping,PP_ROTATION_NONE,p->ii->crop_hpan,p->ii->crop_vpan);
				int dw=fit->width;
				int dh=fit->height;
				
				if(dw > p->width)
					dw=p->width;

				if(dh > p->height)
					dh=p->height;

				if(dw>gdk_pixbuf_get_width(transformed))
					dw=gdk_pixbuf_get_width(transformed);

				if(dh>gdk_pixbuf_get_height(transformed))
					dh=gdk_pixbuf_get_height(transformed);

				if(p->ii->mask)
				{
					transformed=gdk_pixbuf_copy(transformed);
					maskpixbuf(transformed,fit->xoffset,fit->yoffset,dw,dh,p->ii->mask,
						p->ii->layout.bgcol.red>>8,p->ii->layout.bgcol.green>>8,p->ii->layout.bgcol.blue>>8);
				}

				gdk_draw_pixbuf(p->widget->window,NULL,transformed,
					fit->xoffset,fit->yoffset,
					fit->xpos,fit->ypos,
					dw,dh,
					GDK_RGB_DITHER_NONE,0,0);

				p->ii->SetHRPreview(transformed);

				if(p->ii->mask)
					g_object_unref(transformed);

				// If drawing the high-res preview obliterates any gridlines we can repair them here.
				p->ii->layout.DrawGridLines(p->widget);

				delete fit;
			}
			else
				Debug[TRACE] << "RenderHRJob cancelled - detected from main thread" << endl;

			if(p->ii->hrrenderjob==p)
				p->ii->hrrenderjob=NULL;

			Debug[TRACE] << "Main thread callback complete - signalling subthread" << endl;
		}
		catch(const char *err)
		{
			Debug[WARN] << "HRRender main thread function caught error: " << err << endl;
		}
		p->sync.Broadcast();

		return(FALSE);
	}

	protected:
	Layout_ImageInfo *ii;
	GtkWidget *widget;
	int xpos,ypos;
	int width,height;
	CachedImage *transformed;
//	GdkPixbuf *transformed;
	ThreadSync sync;
};

#if 0
// Subthread for rendering high-resolution previews.
// This code uses the subthread to create a GdkPixbuf from
// an image, then defers to the main thread to perform the
// actual rendering.

class hr_payload : public PTMutex, public ThreadFunction
{
	public:
	hr_payload(ProfileManager *p,CMTransformFactory *f,Layout_ImageInfo *ii,GtkWidget *wid,int x,int y,int w,int h)
		: PTMutex(), ThreadFunction(), profman(p), factory(f), ii(ii), widget(wid),
		xpos(x), ypos(y), width(w), height(h), transformed(NULL), thread(this)
	{
		thread.Start();
		thread.WaitSync();
	}
	~hr_payload()
	{
	}
	void Stop()
	{
		thread.Stop();
	}
	static bool testbreakfunc(void *ud)
	{
		Thread *t=(Thread *)ud;
		return(t->TestBreak());
	}
	virtual int Entry(Thread &t)
	{
		ObtainMutex();	// We use this to avoid race conditions when cleaning up.

		// To avoid a deadlock situation if, say, the Apply Profile dialog is open and GTK decides that
		// now is a good time to redraw the window, we only attempt the mutex, and bail out if it fails.
		int count=10;
		while(!ii->AttemptMutexShared())
		{
			if(count==0)
			{
				Debug[WARN] << "Giving up attempt on mutex - bailing out" << endl;
				// The calling thread is waiting for us to acknowledge startup, so we have to send
				// the Sync before bailing out.
				t.SendSync();
				g_timeout_add(1,hr_payload::CleanupFunc,this);
				ReleaseMutex();
				return(0);
			}
#ifdef WIN32
			Sleep(50);
#else
			usleep(50000);
#endif
			--count;
		}

		t.SendSync();

		// We sleep briefly before doing anything time-intensive - that way we can bail out rapidly
		// if the user's doing something heavily interactive, like panning an image, or resizing the window

		for(int i=0;i<75;++i)
		{
#ifdef WIN32
			Sleep(10);
#else
			usleep(10000);
#endif
			if(t.TestBreak())
			{
//				Debug[TRACE] << "Got break signal while pausing - Releasing" << endl;
				ii->ReleaseMutex();
				g_timeout_add(1,hr_payload::CleanupFunc,this);
				ReleaseMutex();
				return(0);
			}
		}

		if(t.TestBreak())
		{
//			Debug[TRACE] << "Subthread releasing mutex and cancelling" << endl;
			ii->ReleaseMutex();
			g_timeout_add(1,hr_payload::CleanupFunc,this);
			ReleaseMutex();
			return(0);
		}

		try
		{
			CMSProfile *targetprof;

			CMColourDevice tdev=CM_COLOURDEVICE_NONE;
			if((targetprof=profman->GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
				tdev=CM_COLOURDEVICE_PRINTERPROOF;
			else if((targetprof=profman->GetProfile(CM_COLOURDEVICE_DISPLAY)))
				tdev=CM_COLOURDEVICE_DISPLAY;
			else if((targetprof=profman->GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
				tdev=CM_COLOURDEVICE_DEFAULTRGB;
			if(targetprof)
				delete targetprof;

			if(t.TestBreak())
			{
//				Debug[TRACE] << "Subthread releasing mutex and cancelling" << endl;
				ii->ReleaseMutex();
				g_timeout_add(1,hr_payload::CleanupFunc,this);
				return(0);
			}

//			Debug[TRACE] << "Generating high-res preview - Using tdev: " << tdev << endl;

			ImageSource *is=ii->GetImageSource(tdev,factory);

//			Debug[TRACE] << "Got imagesource - fitting and rendering" << endl;

			LayoutRectangle r(is->width,is->height);
			LayoutRectangle target(xpos,ypos,width,height);

			RectFit *fit=r.Fit(target,ii->allowcropping,ii->rotation,ii->crop_hpan,ii->crop_vpan);

			if(fit->rotation)
			{
				ImageSource_Interruptible *ii=new ImageSource_Rotate(is,fit->rotation);
				ii->SetTestBreak(testbreakfunc,&t);
				is=ii;
			}
			is=ISScaleImageBySize(is,fit->width,fit->height,IS_SCALING_AUTOMATIC);
			delete fit;
			// We create new Fit in the idle-function because the hpan/vpan may have changed.

			ProgressThread prog(t);
			transformed=pixbuf_from_imagesource(is,ii->layout.bgcol.red>>8,ii->layout.bgcol.green>>8,ii->layout.bgcol.blue>>8,&prog);

			delete is;

//			Debug[TRACE] << "finished - finalising" << endl;

			if(transformed)
			{
				// Now we defer to the main thread...
				// We add this as a high-priority event because we want it to be
				// run as soon as possible, and within a gtk_main_iteration() loop
				// if necessary.
	//			g_idle_add_full(G_PRIORITY_HIGH,hr_payload::IdleFunc,p,NULL);
				g_timeout_add(1,hr_payload::IdleFunc,this);

				// And wait for the main thread to have rendered the preview
				// Because the rendering will be done via a GTK event callback, there's
				// a potential deadlock here if the main app attempts to delete this ImageInfo
				// between the main thread having completed and the idle function being
				// triggered to draw the thumbnail.  For this reason we'll have to use a
				// tie-break in the ImageInfo destructor.
	//			Debug[TRACE] << "Waiting for all-clear from main thread..." << endl;
	//			t->WaitSync();
			}
			else
			{
//				Debug[TRACE] << "Thread cancelled" << endl;
				g_timeout_add(1,hr_payload::CleanupFunc,this);
				ii->ReleaseMutex();
				ReleaseMutex();
				return(0);
			}
		}
		catch (const char *err)
		{
//			Debug[TRACE] << "Subthread caught exception: " << err << endl;
			g_timeout_add(1,hr_payload::CleanupFunc,this);
		}
		Debug[COMMENT] << "Subthread waiting for main thread to finish drawing" << endl;
		thread.WaitSync();
		Debug[COMMENT] << "Subthread releasing mutex and exiting" << endl;
		ii->ReleaseMutex();
		ReleaseMutex();
		return(0);
	}

	// CleanupFunc - runs on the main thread's context.
	static gboolean CleanupFunc(gpointer ud)
	{
		hr_payload *p=(hr_payload *)ud;
//		Debug[TRACE] << "Main thread sending sync signal" << endl;
		p->thread.SendSync();

		// There's a race condition here.  Once we send this signal the subthread will
		// release the mutex - but it's possible this function will have deleted the object first.
		// To avoid this, we obtain the mutex here, then release it again.
		// (In actual fact this race condition should be avoided by the fact that
		// this class's destructor deletes the thread - thus the subthread should be
		// guaranteed to have exited before this class is deleted.)

//		Debug[TRACE] << "Thread cleanup - race prevention - obtaining mutex from thread " << p->thread.GetThreadID() << endl;
		p->ObtainMutex();
//		Debug[TRACE] << "Thread cleanup - race prevention - releasing mutex" << endl;
		p->ReleaseMutex();
		Debug[TRACE] << "Done" << endl;

		// We clear the renderthread pointer in the ImageInfo here before deleting it
		// to avoid the main thread trying to cancel it after deletion.
		// This should be safe since this function runs on the main thread's context.
		if(p->ii->hrrenderthread==p)
			p->ii->hrrenderthread=NULL;

		delete p;
		return(FALSE);
	}

	// IdleFunc - runs on the main thread's context,
	// thus, can safely render into the UI.
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

		if(!p->thread.TestBreak())
		{
			if(p->ii->hrrenderthread==p)
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
				dw=gdk_pixbuf_get_width(p->transformed);
			}

			if(dh>gdk_pixbuf_get_height(p->transformed))
			{
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
//		Debug[TRACE] << "Preview drawn - sending sync to sub-thread" << endl;
		p->thread.SendSync();

		// There's a race condition here.  Once we send this signal the subthread will
		// release the mutex - but it's possible this function will have deleted the object first.
		// To avoid this, we obtain the mutex here, then release it again.
		// (In actual fact this race condition should be avoided by the fact that
		// this class's destructor deletes the thread - thus the subthread should be
		// guaranteed to have exited before this class is deleted.)

//		Debug[TRACE] << "Thread cleanup - race prevention - obtaining mutex from thread " << p->thread.GetThreadID() << endl;
		p->ObtainMutex();
		Debug[COMMENT] << "Thread cleanup - race prevention - releasing mutex" << endl;
		p->ReleaseMutex();
//		Debug[TRACE] << "Done" << endl;

		// We clear the renderthread pointer in the ImageInfo here before deleting it
		// to avoid the main thread trying to cancel it after deletion.
		// This should be safe since this function runs on the main thread's context.
		if(p->ii->hrrenderthread==p)
			p->ii->hrrenderthread=NULL;

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
	Thread thread;
};
#endif

void Layout_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	GdkPixbuf *thumbnail=hrpreview;
	GdkPixbuf *transformed=NULL;
	RectFit *fit=NULL;
	int dw,dh;

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
			dw=gdk_pixbuf_get_width(thumbnail);

		if(dh>gdk_pixbuf_get_height(thumbnail))
			dh=gdk_pixbuf_get_height(thumbnail);

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
			dw=gdk_pixbuf_get_width(transformed);

		if(dh>gdk_pixbuf_get_height(transformed))
			dh=gdk_pixbuf_get_height(transformed);

		if(mask)
			maskpixbuf(transformed,fit->xoffset,fit->yoffset,dw,dh,mask,
				layout.bgcol.red>>8,layout.bgcol.green>>8,layout.bgcol.blue>>8);
		thumbnail=transformed;

		// Trigger a rendering thread if there isn't one already
		// and if high-res previews are enabled
		if(hrrenderjob==NULL && layout.state.FindInt("HighresPreviews"))
		{
			if(width>192 && height>192) // Generating lots of thumbs simultaneously is expensive, and if the images are small,
			{							// highres previews are of limited value anyway.
//				hrrenderthread=new hr_payload(&layout.state.profilemanager,layout.factory,this,widget,xpos,ypos,width,height);

				hrrenderjob=new HRRenderJob(this,widget,xpos,ypos,width,height);
				layout.jobdispatcher.DeleteCompleted();
				layout.jobdispatcher.AddJob(hrrenderjob);
			}
		}
	}

	gdk_draw_pixbuf(widget->window,NULL,thumbnail,
		fit->xoffset,fit->yoffset,
		fit->xpos,fit->ypos,
		dw,dh,
		GDK_RGB_DITHER_NONE,0,0);

	if(transformed)
		g_object_unref(transformed);

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

	if(maskfilename && !mask)
	{
		mask=egg_pixbuf_get_thumbnail_for_file (maskfilename, EGG_PIXBUF_THUMBNAIL_LARGE, &err);
//		Debug[TRACE] << "Attempting to load mask from: " << maskfilename << endl;
		if(!mask)
		{
			Debug[WARN] << "Mask loading failed - trying ImageSource method" << endl;
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
				Debug[ERROR] << "Error: " << err << endl;
			}	
			if(!mask)
			{
				if(err && err->message)
					Debug[ERROR] << "Error: " << err->message << endl;
				else
					Debug[ERROR] << "Can't get mask thumbnail" << endl;
				free(maskfilename);
				maskfilename=NULL;
			}
		}
	}

	Debug[TRACE] << "Thumbnail not cached - loading..." << endl;

	ImageSource *src=NULL;
		
	thumbnail=egg_pixbuf_get_thumbnail_for_file (filename, EGG_PIXBUF_THUMBNAIL_LARGE, &err);

	if(!thumbnail)
	{
		Debug[WARN] << "Can't get pixbuf - loading thumbnail via ImageSource..." << endl;
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
//	Debug[TRACE] << "Checking for Display Profile..." << endl;
	CMSProfile *targetprof;
	CMColourDevice target=CM_COLOURDEVICE_NONE;
	if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
		target=CM_COLOURDEVICE_PRINTERPROOF;
	else if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
		target=CM_COLOURDEVICE_DISPLAY;
	else if((targetprof=layout.state.profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
		target=CM_COLOURDEVICE_DEFAULTRGB;
	if(targetprof)
		delete targetprof;

	if(target!=CM_COLOURDEVICE_NONE)
	{
		if(!src)
		{
			src=ISLoadImage(filename);
		}

		RefCountPtr<CMSTransform> transform;
		if(src)
		{
			// FIXME - use refcounted pointers here to solve lifespan issue.
			RefCountPtr<CMSProfile> emb;
			if(customprofile)
				emb=layout.factory->GetManager().GetProfile(customprofile);  // FIXME: lifespan?
			else
				emb=src->GetEmbeddedProfile();
			if(emb)
			{
//				Debug[TRACE] << "Creating embedded->monitor transform..." << endl;
				if(emb->GetColourSpace()!=IS_TYPE_RGB)
				{
//					Need to replace the RGB thumbnail with a CMYK or Greyscale version!
					Debug[TRACE] << "Creating new thumbnail - CMYK->monitor" << endl;
					int w,h;
					w=(src->width*256)/src->height;
					h=256;
					if(w>256)
					{
						w=256;
						h=(src->height*256)/src->width;
					}
					src=ISScaleImageBySize(src,w,h,IS_SCALING_NEARESTNEIGHBOUR);
					if((transform = layout.factory->GetTransform(target,&*emb,customintent)))
						src=new ImageSource_CMS(src,transform);
					thumbnail=pixbuf_from_imagesource(src);
					delete src;
					src=NULL;
					transform=NULL; // Don't want to apply the transform a second time...
				}
				else
				{
					transform = layout.factory->GetTransform(target,&*emb,customintent);
				}
			}
			else
			{
//				Debug[TRACE] << "Creating default->monitor transform..." << endl;
				transform = layout.factory->GetTransform(target,IS_TYPE_RGB,customintent);
			}
		}
		if(transform)
		{
//			Debug[TRACE] << "Applying transform..." << endl;
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

	Debug[TRACE] << "done" << endl;

	return(thumbnail);
}


LayoutRectangle *Layout_ImageInfo::GetBounds()
{
	// Dummy function - override in subclasses!
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
	try
	{
		is=ApplyEffects(is,PPEFFECT_PRESCALE);

		IS_TYPE colourspace=layout.GetColourSpace(target);

		if(STRIP_ALPHA(is->type)==IS_TYPE_GREY && !is->GetEmbeddedProfile())
			is=new ImageSource_Promote(is,colourspace);

		if(STRIP_ALPHA(is->type)==IS_TYPE_BW)
			is=new ImageSource_Promote(is,colourspace);

		// If this fails we don't bother with the histogram, since another thread has it
		// locked for writing.

		if(histogram.AttemptMutexShared())
		{
			is=new PPIS_Histogram(is,histogram);
			histogram.ReleaseMutexShared();	// ReleaseShared because the Histogram itself holds an exclusive lock
											// and we don't want to cancel its exclusivity!
		}

		RefCountPtr<CMSTransform> transform;

		if(factory)
		{
			RefCountPtr<CMSProfile> emb;
			if(customprofile)
				emb=factory->GetManager().GetProfile(customprofile);  // FIXME: Lifespan!
			else
				emb=is->GetEmbeddedProfile();
			
			if(emb)
			{
	//				Debug[TRACE] << "Has embedded / assigned profile..." << endl;
				transform=factory->GetTransform(target,&*emb,customintent);  // FIXME: intent!
			}
			else
			{
	//				Debug[TRACE] << "No embedded profile - using default" << endl;
				transform=factory->GetTransform(target,IS_TYPE(STRIP_ALPHA(is->type)),customintent);
			}

			if(transform)
				is=new ImageSource_CMS(is,transform);
		}
	}
	catch(const char *err)
	{
		if(is)
		{
			Debug[TRACE] << "Layout_ImageInfo::GetImageSource bailing out and deleting results so far..." << endl;
			delete is;
		}
		throw err;
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
//			Debug[TRACE] << "Rotating mask" << endl;
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
	if(hrrenderjob)
	{
		layout.jobdispatcher.CancelJob(hrrenderjob);
//		hrrenderthread->Stop();	// We don't actually delete it here - the thread is responsible for its own
								// demise (by way of a GTK Idle function running on the main thread's context)
								// but having signalled it to stop, we can discard this pointer to it.
	}
//	hrrenderthread=NULL;
	hrrenderjob=NULL;
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


// Because the only reason you would need to ObtainMutex() the ImageInfo (rather than ObtainShared())
// is to make a write-change to it, we flush the high-res preview on the assumption that the change
// will invalidate it.  Note, also, we won't be able to obtain the exclusive lock while the thread's
// running.
// FIXME - would be better to require an explicit flush() of some kind.
void Layout_ImageInfo::ObtainMutex()
{
//	Debug[TRACE] << "In custom Obtain method - flushing preview..." << endl;
	FlushHRPreview();
//	Debug[TRACE] << "Now attempting to obtain exclusive lock..." << endl;
	while(!PPEffectHeader::AttemptMutex())
	{
//		Debug[TRACE] << "Can't get exclusive lock - performing main loop iteration" << endl;
		gtk_main_iteration();
	}
//	Debug[TRACE] << "Done" << endl;
}


PPHistogram &Layout_ImageInfo::GetHistogram()
{
	return(histogram);
}
