/*
 * layout_single.cpp - Has responsibility for tracking the list of images and their layout.
 * Also has responsibility for building the ImageSource stack at print time.
 *
 * Copyright (c) 2004 - 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2004-12-24: Now opens the imagesource when placing an image, since we need both the
 *             natural size of the image (which the thumbnail code could provide)
 *             and the resolution of the image (which the thumbnail code can't provide).
 *             Getting the actual imagesource (with CMS transform) from the ImageInfo
 *             is now delegated to the superclasses.
 *
 * 2005-08-21: Added allowcropping and allowrotation parameters to allow for the menu
 *             generalisation.
 *
 * 2008-09-25: Opening the imagesource and fetching the resolution is now handled
 *             by the Layout_ImageInfo superclass, since the UI needs to present
 *             physical size information for all layout types.
 */

#include <iostream>
#include <string.h>

#include "miscwidgets/generaldialogs.h"
#include "dialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "imageutils/rotatepixbuf.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_crop.h"
#include "imagesource/imagesource_rotate.h"
#include "photoprint_state.h"
#include "pp_layout_single.h"

#include "layout_single.h"

using namespace std;


ConfigTemplate Layout_SingleDB::Template[]=
{
	ConfigTemplate("LeftMargin",10),
	ConfigTemplate("RightMargin",10),
	ConfigTemplate("TopMargin",10),
	ConfigTemplate("BottomMargin",10),
	ConfigTemplate()
};


void Layout_Single_ImageInfo::Init()
{
	Layout_Single *l=(Layout_Single *)&layout;
	
	l->GetImageableArea();
	float sw=(width*72.0)/xres;
	float sh=(height*72.0)/yres;
	int mm=(l->imageablewidth>l->imageableheight);
	mm^=width>height;
	if(mm)
	{
		rotation=PP_ROTATION_90;
		sw=(height*72.0)/yres;
		sh=(width*72.0)/xres;
	}
	else
		rotation=PP_ROTATION_NONE;
	float s=100.0;
	float s2=100.0;
	if(sw>l->imageablewidth)
		s=(100.0*l->imageablewidth)/sw;
	if(sh>l->imageableheight)
		s2=(100.0*l->imageableheight)/sh;

	if(s2<s)
		s=s2;
	
	hscale=s;
	vscale=s;
}


Layout_Single_ImageInfo::Layout_Single_ImageInfo(Layout_Single &layout, const char *filename,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,filename,page,allowcropping,rotation), hscale(100), vscale(100)
{
	Init();
}


Layout_Single_ImageInfo::Layout_Single_ImageInfo(Layout_Single &layout, Layout_ImageInfo *ii,int page)
	: Layout_ImageInfo((Layout &)layout,ii,page), hscale(100), vscale(100)
{
	Init();
}


Layout_Single_ImageInfo::~Layout_Single_ImageInfo()
{
}


void Layout_Single_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int dwidth,int dheight)
{
	cerr << "Drawing thumbnail" << endl;
	GdkPixbuf *thumbnail=GetThumbnail();
	GdkPixbuf *transformed=NULL;

	Layout_Single *l=(Layout_Single *)&layout;

	double xr=(xres*100.0)/hscale;
	double yr=(yres*100.0)/vscale;

	int w=0,h=0;
	cerr << "rotation " << rotation << endl;
	switch(rotation)
	{
		case PP_ROTATION_AUTO:
		case PP_ROTATION_NONE:
			{
				double pw=width*72; pw/=xr;
				double ph=height*72; ph/=yr;
				w=int(pw*dwidth/l->pagewidth);
				h=int(ph*dheight/l->pageheight);
				transformed=gdk_pixbuf_scale_simple(thumbnail,w,h,GDK_INTERP_NEAREST);
			}
			break;
		case PP_ROTATION_90:
			{
				double pw=width*72; pw/=yr;
				double ph=height*72; ph/=xr;
				GdkPixbuf *tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
				w=int(ph*dwidth/l->pagewidth);
				h=int(pw*dheight/l->pageheight);
				transformed=gdk_pixbuf_scale_simple(tmp,w,h,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
			}
			break;
		case PP_ROTATION_180:
			{
				double pw=width*72; pw/=yr;
				double ph=height*72; ph/=xr;
				GdkPixbuf *tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_UPSIDEDOWN);
				w=int(pw*dwidth/l->pagewidth);
				h=int(ph*dheight/l->pageheight);
				transformed=gdk_pixbuf_scale_simple(tmp,w,h,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
			}
			break;
		case PP_ROTATION_270:
			{
				double pw=width*72; pw/=yr;
				double ph=height*72; ph/=xr;
				GdkPixbuf *tmp=gdk_pixbuf_rotate_simple(thumbnail,GDK_PIXBUF_ROTATE_CLOCKWISE);
				w=int(ph*dwidth/l->pagewidth);
				h=int(pw*dheight/l->pageheight);
				transformed=gdk_pixbuf_scale_simple(tmp,w,h,GDK_INTERP_NEAREST);
				g_object_unref(G_OBJECT(tmp));
			}
			break;
	}
	l->GetImageableArea();

	int iw=(l->imageablewidth*dwidth)/l->pagewidth;
	if(w>iw)
		w=iw;

	int ih=(l->imageableheight*dheight)/l->pageheight;
	if(h>ih)
		h=ih;

	int lm=(l->leftmargin*dwidth)/l->pagewidth;
	int tm=(l->topmargin*dheight)/l->pageheight;

	gdk_draw_pixbuf(widget->window,NULL,transformed,
		0,0,
		xpos+lm,ypos+tm,
		w,h,
		GDK_RGB_DITHER_NONE,0,0);

	g_object_unref(transformed);
	cerr << "Finished drawing" << endl;
}


ImageSource *Layout_Single_ImageInfo::GetImageSource(CMColourDevice target,CMTransformFactory *factory)
{
	ImageSource *is=Layout_ImageInfo::GetImageSource(target,factory);
	is->xres=(xres*100)/hscale;
	is->yres=(yres*100)/vscale;
	return(is);
}


LayoutRectangle *Layout_Single_ImageInfo::GetBounds()
{
	cerr << "Pixel width: " << width << endl;
	cerr << "HScale: " << hscale << endl;
	cerr << "XRes: " << xres << endl;
	cerr << "Pixel height: " << height << endl;
	cerr << "VScale: " << vscale << endl;
	cerr << "YRes: " << yres << endl;
	float w,h;
	switch(rotation)
	{
		case PP_ROTATION_90:
		case PP_ROTATION_270:
			cerr << "Rotated" << endl;
			w=(width*72*vscale)/(xres*100);
			h=(height*72*hscale)/(yres*100);
			break;

		default:
			cerr << "No rotation" << endl;
			w=(width*72*hscale)/(xres*100);
			h=(height*72*vscale)/(yres*100);
			break;
	}

	return(new LayoutRectangle(0,0,w,h));
}


RectFit *Layout_Single_ImageInfo::GetFit(double scale)
{
	LayoutRectangle *r=GetBounds();
	RectFit *result=new RectFit;
	result->xpos=result->ypos=result->xoffset=result->yoffset=0;
	result->width=r->w;
	result->height=r->h;
	result->scale=1.0;
	delete r;
	return(result);
}


Layout_Single::Layout_Single(PhotoPrint_State &state,Layout *oldlayout)
	: Layout(state,oldlayout)
{
}


Layout_Single::~Layout_Single()
{
}


void Layout_Single::Reflow()
{
	int page=0;
	LayoutIterator it(*this);
	Layout_Single_ImageInfo *ii=(Layout_Single_ImageInfo *)it.FirstImage();
	while(ii)
	{
		ii->page=page;
		ii=(Layout_Single_ImageInfo *)it.NextImage();
		++page;
	}
	if(page<1)
		page=1;
	pages=page;
	if(currentpage>=page)
		currentpage=page-1;
}


int Layout_Single::AddImage(const char *filename,bool allowcropping,PP_ROTATION rotation)
{
	int page=pages+1;
	for(int i=0;i<=pages;++i)
	{
		if(!ImageAt(i))
		{
			cerr << "No image found at page " << i << endl;
			page=i;
			i=pages;
		}
	}
	cerr << "Adding image to page " << page << endl;
	Layout_Single_ImageInfo *ii=NULL;
	try
	{
		ii=new Layout_Single_ImageInfo(*this,filename,page,false,rotation);
	}
	catch(const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
	if(ii)
	{
		imagelist.push_back(ii);

		if(page>=pages)
			++pages;
	}
	else
		page=currentpage;
	return(page);
}


void Layout_Single::CopyImage(Layout_ImageInfo *ii)
{
	int page=pages+1;
	for(int i=0;i<=pages;++i)
	{
		if(!ImageAt(i))
		{
			cerr << "No image found at page " << i << endl;
			page=i;
			i=pages;
		}
	}
	ii=new Layout_Single_ImageInfo(*this,ii,page);
	imagelist.push_back(ii);
	if(page>=pages)
		++pages;
}


ImageSource *Layout_Single::GetImageSource(int page,CMColourDevice target,CMTransformFactory *factory,int res,bool completepage)
{
	ImageSource *result=NULL;
	try
	{
		Layout_Single_ImageInfo *ii=(Layout_Single_ImageInfo *)ImageAt(page);
		if(ii)
		{
			ImageSource *is=ii->GetImageSource(target,factory);
			switch(ii->rotation)
			{
				case PP_ROTATION_90:
					is=new ImageSource_Rotate(is,90);
					break;
				case PP_ROTATION_180:
					is=new ImageSource_Rotate(is,180);
					break;
				case PP_ROTATION_270:
					is=new ImageSource_Rotate(is,270);
					break;
				default:
					break;
			}
			xoffset=leftmargin;
			yoffset=topmargin;
			GetImageableArea();
			int iw=int((imageablewidth*is->xres)/72.0);
			int ih=int((imageableheight*is->yres)/72.0);
			if((iw<is->width) || (ih<is->height))
			{
				is=new ImageSource_Crop(is,0,0,iw,ih);
			}
			return(is);
		}
	}
	catch (const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
	return(result);
}


void Layout_Single::SetPageExtent(PageExtent &pe)
{
	pe.GetImageableArea();
	pagewidth=pe.pagewidth;
	pageheight=pe.pageheight;
	leftmargin=pe.leftmargin;
	rightmargin=pe.rightmargin;
	topmargin=pe.topmargin;
	bottommargin=pe.bottommargin;
	GetImageableArea();
}


void Layout_Single::DBToLayout(LayoutDB &db)
{
	Layout::DBToLayout(db);

	SetPageExtent(state.printer);
	SetMargins(db.singledb.FindInt("LeftMargin"),db.singledb.FindInt("RightMargin"),
		db.singledb.FindInt("TopMargin"),db.singledb.FindInt("BottomMargin"));
}


void Layout_Single::LayoutToDB(LayoutDB &db)
{
	Layout::LayoutToDB(db);

	db.singledb.SetInt("LeftMargin",leftmargin);
	db.singledb.SetInt("RightMargin",rightmargin);
	db.singledb.SetInt("TopMargin",topmargin);
	db.singledb.SetInt("BottomMargin",bottommargin);
}


Layout_Single_ImageInfo *Layout_Single::ImageAt(int page)
{
	Layout_Single_ImageInfo *result=NULL;
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		Layout_Single_ImageInfo *nii=(Layout_Single_ImageInfo *)ii;
		if(nii->page==page)
		{
			result=nii;
		}
		ii=it.NextImage();
	}
	return(result);
}


GtkWidget *Layout_Single::CreateWidget()
{
	return(pp_layout_single_new(&state));
}


void Layout_Single::RefreshWidget(GtkWidget *widget)
{
	pp_layout_single_refresh(PP_LAYOUT_SINGLE(widget));
}


const char *Layout_Single::GetType()
{
	return("Single");
}


void (*Layout_Single::SetUnitFunc())(GtkWidget *wid,enum Units unit)
{
	return(pp_layout_single_set_unit);
}


int Layout_Single::GetCapabilities()
{
	return(PPLAYOUT_ROTATE|PPLAYOUT_MASK|PPLAYOUT_EFFECTS|PPLAYOUT_PROFILE);
}


// We override this to set the top/left margin
void Layout_Single::Print(Progress *p)
{
	xoffset=leftmargin;
	yoffset=topmargin;
	Layout::Print(p);
}


bool Layout_Single_ImageInfo::GetSelected()
{
	Layout_Single *l=(Layout_Single *)&layout;
	return(page==l->currentpage);
}


#if 0
class LayoutIterator_Single : public LayoutIterator
{
	LayoutIterator_Single(Layout_Single &header) : LayoutIterator(header)
	{
	}
	virtual ~LayoutIterator_Single()
	{
	}
	virtual Layout_ImageInfo *FirstSelected()
	{
		Layout_ImageInfo *ii=FirstImage();
		while(ii)
		{
			if(ii->page==header.currentpage)
				return(ii);
			ii=NextSelected();
		}
		return(NULL);
	}
	virtual Layout_ImageInfo *NextSelected()
	{
		Layout_ImageInfo *ii=NextImage();
		while(ii)
		{
			if(ii->page==header.currentpage)
				return(ii);
			ii=NextSelected();
		}
		return(NULL);
	}
};

LayoutIterator Layout_Single::GetIterator()
{
	return(LayoutIterator_Single(*this));
}
#endif

#if 0
// Re-implement this with a subclass of LayoutIterator

Layout_ImageInfo *Layout_Single::FirstSelected()
{
	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		if(ii->page==currentpage)
			return(ii);
		ii=NextSelected();
	}
	return(NULL);
}


Layout_ImageInfo *Layout_Single::NextSelected()
{
	Layout_ImageInfo *ii=NextImage();
	while(ii)
	{
		if(ii->page==currentpage)
			return(ii);
		ii=NextSelected();
	}
	return(NULL);
}

#endif
