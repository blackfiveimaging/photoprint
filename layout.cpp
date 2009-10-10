/*
 * layout.cpp - A base class for layouts.
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

#include "imagesource/imagesource.h"
#include "imagesource/imagesource_gdkpixbuf.h"
#include "imagesource/imagesource_cms.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_mask.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_promote.h"

#include "photoprint_state.h"
#include "support/progress.h"
#include "support/util.h"
#include "support/layoutrectangle.h"

#include "layout.h"

using namespace std;


ConfigTemplate LayoutDB::Template[]=
{
	ConfigTemplate("LayoutType","NUp"),
	ConfigTemplate("AllowCropping",int(0)),
	ConfigTemplate("Rotation",int(PP_ROTATION_AUTO)),
	ConfigTemplate()
};


LayoutIterator::LayoutIterator(Layout &header) : header(header)
{
}

LayoutIterator::~LayoutIterator()
{
}

Layout_ImageInfo *LayoutIterator::FirstImage()
{
	iterator=header.imagelist.begin();
	if(iterator==header.imagelist.end())
		return(NULL);
	return(*iterator);
}

Layout_ImageInfo *LayoutIterator::NextImage()
{
	if(iterator==header.imagelist.end())
		return(NULL);
	++iterator;
	if(iterator==header.imagelist.end())
		return(NULL);
	return(*iterator);
}

Layout_ImageInfo *LayoutIterator::FirstSelected()
{
	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		if(ii->GetSelected())
			return(ii);
		ii=NextSelected();
	}
	return(NULL);
}

Layout_ImageInfo *LayoutIterator::NextSelected()
{
	Layout_ImageInfo *ii=NextImage();
	while(ii)
	{
		if(ii->GetSelected())
			return(ii);
		ii=NextSelected();
	}
	return(NULL);
}


Layout::Layout(PhotoPrint_State &state,Layout *oldlayout)
	: PageExtent(), state(state), xoffset(0), yoffset(0), pages(1), currentpage(0), backgroundfilename(NULL), background(NULL),
	backgroundtransformed(NULL), imagelist(NULL), factory(NULL), gc(NULL)
{
	factory=state.profilemanager.GetTransformFactory();
}


Layout::~Layout()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		delete ii;
		ii=it.FirstImage();
	}

	if(gc)
		g_object_unref(G_OBJECT(gc));
	if(factory)
		delete factory;
	if(backgroundtransformed)
		g_object_unref(G_OBJECT(backgroundtransformed));
	backgroundtransformed=NULL;

	if(background)
		g_object_unref(G_OBJECT(background));
	background=NULL;

	if(backgroundfilename)
		free(backgroundfilename);
	backgroundfilename=NULL;
}


void Layout::Clear()
{
	SetBackground(NULL);
	FlushHRPreviews();

	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		delete ii;
		ii=it.NextImage();
	}
	pages=1;
}


// DUMMY FUNCTION - should be overridden by subclasses
int Layout::AddImage(const char *filename,bool allowcropping,PP_ROTATION rotation)
{
	cerr << "AddImage: Dummy function - should be overridden" << endl;
	return(0);
}


// DUMMY FUNCTION - should be overridden by subclasses
void Layout::CopyImage(Layout_ImageInfo *ii)
{
	cerr << "CopyImage: Dummy function - should be overridden" << endl;
}


ImageSource *Layout::GetImageSource(int page,CMColourDevice target,CMTransformFactory *factory,int res,bool completepage)
{
	cerr << "GetImageSource: Dummy function - should be overridden" << endl;
	return(NULL);
}


IS_TYPE Layout::GetColourSpace(CMColourDevice target)
{
	enum IS_TYPE colourspace=IS_TYPE_RGB;
	CMSProfile *profile=state.profilemanager.GetProfile(target);
	if(profile)
	{
		colourspace=profile->GetColourSpace();
		delete profile;
	}
	else
	{
		const char *cs=state.FindString("PrintColourSpace");
		if(strcmp(cs,"RGB")==0)
			colourspace=IS_TYPE_RGB;
		else if(strcmp(cs,"CMYK")==0)
			colourspace=IS_TYPE_CMYK;
		else
			cerr << "PrintColourSpace is set to an unknown colour space!" << endl;
	}
	return(colourspace);
}


void Layout::TransferImages(Layout *oldlayout,Progress *p)
{
	if(oldlayout)
	{
		int count=oldlayout->imagelist.size();

		LayoutIterator it(*oldlayout);
		Layout_ImageInfo *ii=it.FirstImage();
		int img=0;
		while(ii)
		{
			CopyImage(ii);
			ii=it.NextImage();
			if(p)
			{
				if(!(p->DoProgress(img,count)))
					ii=NULL;
			}
			++img;
		}
	}
	if(currentpage>=pages)
		currentpage=pages-1;
}


void Layout::UpdatePageSize()
{
	int t=topmargin,l=leftmargin,r=rightmargin,b=bottommargin;
	SetPageExtent(state.printer);
	SetMargins(l,r,t,b);
}


void Layout::DBToLayout(LayoutDB &db)
{
}


void Layout::LayoutToDB(LayoutDB &db)
{
}


void Layout::Reflow()
{
}


GtkWidget *Layout::CreateWidget()
{
	return(NULL);
}


void Layout::RefreshWidget(GtkWidget *widget)
{
}


int Layout::GetCurrentPage()
{
	return(currentpage);
}


void Layout::SetCurrentPage(int page)
{
	currentpage=page;
}


int Layout::FreeSlots()
{
	return(0);
}


void Layout::Print(Progress *p)
{
	state.printer.SetProgress(p);
	CMTransformFactory *factory=state.profilemanager.GetTransformFactory();
	for(int p=0;p<GetPages();++p)
	{
		ImageSource *is=GetImageSource(p,CM_COLOURDEVICE_PRINTER,factory);
		if(is)
		{
			state.printer.Print(is,xoffset,yoffset);
			delete is;
		}
	}
	delete factory;
	state.printer.SetProgress(NULL);
}

void (*Layout::SetUnitFunc())(GtkWidget *wid,enum Units unit)
{
	cerr << "This function should be overridden" << endl;
	return(NULL);
}


void Layout::MakeGC(GtkWidget *widget)
{
	bgcol.pixel=0;
	bgcol.red=65535;
	bgcol.green=65535;
	bgcol.blue=65535;

	bgcol2.pixel=0;
	bgcol2.red=32767;
	bgcol2.green=32767;
	bgcol2.blue=32767;

	// Use printer and monitor profiles to calculate "paper white".
	// If there's no display profile, then we can use the Default RGB profile instead...
//		cerr << "Checking for Display Profile..." << endl;
	CMSProfile *targetprof;
	CMColourDevice target=CM_COLOURDEVICE_NONE;
	if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_PRINTERPROOF)))
		target=CM_COLOURDEVICE_PRINTERPROOF;
	else if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DISPLAY)))
		target=CM_COLOURDEVICE_DISPLAY;
	else if((targetprof=state.profilemanager.GetProfile(CM_COLOURDEVICE_DEFAULTRGB)))
		target=CM_COLOURDEVICE_DEFAULTRGB;
	if(targetprof)
		delete targetprof;


	if(target!=CM_COLOURDEVICE_NONE)
	{
		CMSTransform *transform=NULL;
//			cerr << "Creating default->monitor transform..." << endl;
		transform = factory->GetTransform(target,IS_TYPE_RGB,LCMSWRAPPER_INTENT_DEFAULT);
		if(transform)
		{
//				cerr << "Applying transform..." << endl;

			ISDataType rgbtriple[3];
			rgbtriple[0]=bgcol.red;
			rgbtriple[1]=bgcol.green;
			rgbtriple[2]=bgcol.blue;
			transform->Transform(rgbtriple,rgbtriple,1);
			bgcol.red=rgbtriple[0];
			bgcol.green=rgbtriple[1];
			bgcol.blue=rgbtriple[2];

			rgbtriple[0]=bgcol2.red;
			rgbtriple[1]=bgcol2.green;
			rgbtriple[2]=bgcol2.blue;
			transform->Transform(rgbtriple,rgbtriple,1);
			bgcol2.red=rgbtriple[0];
			bgcol2.green=rgbtriple[1];
			bgcol2.blue=rgbtriple[2];
		}
	}
	gc=gdk_gc_new(widget->window);
}


void Layout::DrawPreviewBorder(GtkWidget *widget)
{
	if(!gc)
		MakeGC(widget);

	gdk_gc_set_rgb_fg_color(gc,&bgcol2);
	gdk_draw_rectangle (widget->window,
		gc,TRUE,
		0,0,
		widget->allocation.width,widget->allocation.height);
}


void Layout::DrawPreviewBG(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	if(!gc)
		MakeGC(widget);

	DrawPreviewBorder(widget);

	gdk_gc_set_rgb_fg_color(gc,&bgcol);
	gdk_draw_rectangle (widget->window,
		gc,TRUE,
		xpos,ypos,
		width,height);

	GetImageableArea();
	LayoutRectangle sr(leftmargin,topmargin,
		imageablewidth,imageableheight);

	double scale=width;
	scale/=pagewidth;
	sr.Scale(scale);

//	gdk_draw_rectangle (widget->window,
//		widget->style->mid_gc[widget->state],FALSE,
//		xpos+sr.x,ypos+sr.y,sr.w,sr.h);

	if(background)
	{
		if(backgroundtransformed)
			g_object_unref(backgroundtransformed);
		backgroundtransformed=NULL;
		
		int bw=gdk_pixbuf_get_width(background);
		int bh=gdk_pixbuf_get_height(background);

		int rotation=0;
		if((bw<bh)^(width<height))
			rotation=90;
	
		GdkPixbuf *tmp;
		switch(rotation)
		{
			case 0:
				backgroundtransformed=gdk_pixbuf_scale_simple(background,width,height,GDK_INTERP_NEAREST);
				break;
			case 90:
				tmp=gdk_pixbuf_rotate_simple(background,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
				backgroundtransformed=gdk_pixbuf_scale_simple(tmp,width,height,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
				break;
		}
	
		gdk_draw_pixbuf(widget->window,NULL,backgroundtransformed,
			0,0,
			xpos,ypos,
			width,height,
			GDK_RGB_DITHER_NONE,0,0);
	}
}


void Layout::DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	DrawPreviewBG(widget,xpos,ypos,width,height);

	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();

	while(ii)
	{
		if(currentpage==ii->page)
			ii->DrawThumbnail(widget,xpos,ypos,width,height);
		ii=it.NextImage();
	}
}


int Layout::GetPages()
{
	return(pages);
}


int Layout::CountSelected()
{
	int count=0;

	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		++count;
		ii=it.NextSelected();
	}
	return(count);
}


void Layout::SelectAll()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		ii->SetSelected(true);
		ii=it.NextImage();
	}
}


void Layout::SelectNone()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		ii->SetSelected(false);
		ii=it.NextSelected();
	}
}


Layout_ImageInfo *Layout::ImageAtCoord(int x,int y)
{
	return(NULL);
}


int Layout::GetCapabilities()
{
	return(0);
}


void Layout::SetBackground(const char *filename)
{
	if(backgroundtransformed)
		g_object_unref(G_OBJECT(backgroundtransformed));
	backgroundtransformed=NULL;

	if(background)
		g_object_unref(G_OBJECT(background));
	background=NULL;

	if(backgroundfilename)
		free(backgroundfilename);
	backgroundfilename=NULL;
	
//	cerr << "Setting background..." << endl;
	if(filename && strlen(filename)>0)
	{
		GError *err=NULL;

		backgroundfilename=strdup(filename);

//		cerr << "Attempting to load background from: " << backgroundfilename << endl;
		background=egg_pixbuf_get_thumbnail_for_file (backgroundfilename, EGG_PIXBUF_THUMBNAIL_LARGE, &err);
		if(!background)
		{
//			cerr << "Failed." << endl;
			try
			{
				ImageSource *src=ISLoadImage(backgroundfilename);
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
					background=pixbuf_from_imagesource(src);
					delete src;
				}
			}
			catch(const char *err)
			{
				cerr << "Error: " << err << endl;
			}	
			if(!background)
			{
				if(err && err->message)
					cerr << "Error: " << err->message << endl;
				else
					cerr << "Can't get mask thumbnail" << endl;
				free(backgroundfilename);
				backgroundfilename=NULL;
			}
		}
	}
}


void Layout::FlushThumbnails()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		ii->FlushThumbnail();
		ii=it.NextImage();	
	}

	// We obtain the mutices to ensure the rendering threads have completed.
	ii=it.FirstImage();
	while(ii)
	{
		ii->ObtainMutex();
		ii=it.NextImage();
	}

	// Can't delete this until after the thumbnails have been flushed,
	// in case any rendering threads are still using a transform owned
	// by the factory.

	delete factory;
	factory=new CMTransformFactory(state.profilemanager);

	// If the profile settings have changed, we'll flush the paper white
	// gc too, so it's regenerate with the new settings.

	if(gc)
		g_object_unref(G_OBJECT(gc));
	gc=NULL;

	// And now we release the mutices.
	ii=it.FirstImage();
	while(ii)
	{
		ii->ReleaseMutex();
		ii=it.NextImage();
	}
}


void Layout::FlushHRPreviews()
{
	// If there are multiple renderthreads running,
	// we can cancel them all without waiting for them
	// to exit - then wait for them in turn while we flush
	// the previews.  Quicker than signal / wait / signal / wait / etc.
	CancelRenderThreads();

	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		ii->FlushHRPreview();
		ii=it.NextImage();	
	}
}


void Layout::CancelRenderThreads()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		ii->CancelRenderThread();
		ii=it.NextImage();	
	}
}


