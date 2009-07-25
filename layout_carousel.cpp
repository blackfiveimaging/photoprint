/*
 * layout_carousel.cpp - Has responsibility for tracking the list of images and their layout.
 * Also has responsibility for building the ImageSource stack at print time.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <string.h>

#include "miscwidgets/generaldialogs.h"
#include "dialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "imageutils/rotatepixbuf.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_cms.h"
#include "imagesource/imagesource_segmentmask.h"
#include "imagesource/imagesource_montage.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_crop.h"
#include "imagesource/imagesource_mask.h"
#include "imagesource/imagesource_gdkpixbuf.h"
#include "imagesource/imagesource_solid.h"

#include "imagesource/pixbuf_from_imagesource.h"

#include "photoprint_state.h"
#include "pp_layout_carousel.h"

#include "layout_carousel.h"

using namespace std;


ConfigTemplate Layout_CarouselDB::Template[]=
{
	ConfigTemplate("LeftMargin",int(0)),
	ConfigTemplate("RightMargin",int(0)),
	ConfigTemplate("TopMargin",int(0)),
	ConfigTemplate("BottomMargin",int(0)),
	ConfigTemplate("InnerRadius",int(0)),
	ConfigTemplate("Segments",int(4)),
	ConfigTemplate("Overlap",int(30)),
	ConfigTemplate("AngleOffset",int(0)),
	ConfigTemplate()
};


Layout_Carousel_ImageInfo::Layout_Carousel_ImageInfo(Layout_Carousel &layout, const char *filename,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,filename,page,allowcropping,rotation)
{
}


Layout_Carousel_ImageInfo::Layout_Carousel_ImageInfo(Layout_Carousel &layout, Layout_ImageInfo *ii,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,ii,page,allowcropping,rotation)
{
}


Layout_Carousel_ImageInfo::~Layout_Carousel_ImageInfo()
{
	Layout_Carousel *c=(Layout_Carousel *)&layout;
	c->FlushPreview();
}


void Layout_Carousel_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	// Carousel is built as a whole, so we don't draw the elements individually.
}


void Layout_Carousel_ImageInfo::FlushThumbnail()
{
	// Because the carousel is built as a whole, we have to flush the cached preview
	// along with any obsolete thumbnails.
	Layout_ImageInfo::FlushThumbnail();
	Layout_Carousel *lc=(Layout_Carousel *)&layout;
	lc->FlushPreview();
}


LayoutRectangle *Layout_Carousel_ImageInfo::GetBounds()
{
	Layout_Carousel *l=(Layout_Carousel *)&layout;
	l->GetImageableArea();
	CircleMontage c(l->imageablewidth,l->imageableheight);
	c.SetSegments(l->segments,l->angleoffset,l->overlap);
	return(c.GetSegmentExtent(segment));
}


Layout_Carousel::Layout_Carousel(PhotoPrint_State &state,Layout *oldlayout)
	: Layout(state,oldlayout), segments(4), overlap(30), angleoffset(0), innerradius(0),
	preview(NULL), prevwidth(0), prevheight(0)
{
}


Layout_Carousel::~Layout_Carousel()
{
	FlushPreview();
}


void Layout_Carousel::Reflow()
{
	int page=0;
	int segment=0;
	Layout_Carousel_ImageInfo *ii=(Layout_Carousel_ImageInfo *)FirstImage();
	while(ii)
	{
		ii->page=page;
		ii->segment=segment;
		ii=(Layout_Carousel_ImageInfo *)NextImage();
		++segment;
	}
	pages=1;
}


int Layout_Carousel::AddImage(const char *filename,bool allowcropping,PP_ROTATION rotation)
{
	// AllowCropping is ignored - cropping is always performed.
	// AllowRotation is ignored - rotation is never allowed.
	int page=0;
	Layout_Carousel_ImageInfo *ii=new Layout_Carousel_ImageInfo(*this,filename,page,true,PP_ROTATION_NONE);
	imagelist=g_list_append(imagelist,ii);
	if(page>=pages)
		++pages;
	FlushPreview();

	int ct=CountImages(page);
	ii->segment=ct-1;
	if(ct>segments)
		SetSegments(ct);

	return(page);
}


void Layout_Carousel::CopyImage(Layout_ImageInfo *ii)
{
	int page=0;
	Layout_Carousel_ImageInfo *cii=new Layout_Carousel_ImageInfo(*this,ii,page);
	imagelist=g_list_append(imagelist,cii);
	if(page>=pages)
		++pages;

	int ct=CountImages(page);
	cii->segment=ct-1;
	if(ct>segments)
		SetSegments(ct);

	FlushPreview();
}


ImageSource *Layout_Carousel::GetImageSource(int page,CMColourDevice target,CMTransformFactory *factory,int res,bool completepage)
{
	ImageSource *result=NULL;
	try
	{
		if(imagelist)
		{
			enum IS_TYPE colourspace=GetColourSpace(target);

			IS_ScalingQuality qual=IS_ScalingQuality(state.FindInt("ScalingQuality"));
			if(!res)
				res=state.FindInt("RenderingResolution");

			ImageSource_Montage *mon=new ImageSource_Montage(colourspace,res);

			GetImageableArea();
			int w=(imageablewidth*res)/72;
			int h=(imageableheight*res)/72;
			int ir=(innerradius*res)/72;

			CircleMontage c(w,h);
			c.SetSegments(segments,angleoffset,overlap);
			c.SetInnerRadius(ir);

			ImageSource *source=NULL;
			ImageSource *mask=NULL;
			CMSegment *targetseg=NULL;

			cerr << "Layout_carousel: Left margin: " << leftmargin << endl;
	
			for(int s=0;s<segments;s+=2)
			{
				Layout_ImageInfo *img=GetNthImage(page,s);

				if(img)
				{
					source=img->GetImageSource(target,factory);
					LayoutRectangle r(source->width,source->height);
	
					targetseg=c.GetSegmentExtent(s);
			
					RectFit *fit=r.Fit(*targetseg,true,PP_ROTATION_NONE,img->crop_hpan,img->crop_vpan);
			
					source=ISScaleImageBySize(source,fit->width,fit->height,qual);
	
					mask=new ImageSource_SegmentMask(targetseg,true);
					source=new ImageSource_Crop(source,fit->xoffset,fit->yoffset,mask->width,mask->height);

					source=new ImageSource_Mask(source,mask);
					mon->Add(source,(leftmargin*res)/72+targetseg->x,(topmargin*res)/72+targetseg->y);

					delete fit;
				}
			}
	
			for(int s=1;s<segments;s+=2)
			{
				Layout_ImageInfo *img=GetNthImage(page,s);

				if(img)
				{
					source=img->GetImageSource(target,factory);
					LayoutRectangle r(source->width,source->height);
	
					targetseg=c.GetSegmentExtent(s);
			
					RectFit *fit=r.Fit(*targetseg,true,PP_ROTATION_NONE,img->crop_hpan,img->crop_vpan);
			
					source=ISScaleImageBySize(source,fit->width,fit->height,qual);
			
					mask=new ImageSource_SegmentMask(targetseg,false);
					source=new ImageSource_Crop(source,fit->xoffset,fit->yoffset,mask->width,mask->height);
			
					source=new ImageSource_Mask(source,mask);
					mon->Add(source,(leftmargin*res)/72+targetseg->x,(topmargin*res)/72+targetseg->y);
			
					delete fit;
				}
			}

			result=mon;

			if(completepage)
			{
				// If the completepage flag is set we need to render a solid background.
				// This will be used for print preview and TIFF/JPEG export.

				IS_TYPE colourspace=GetColourSpace(target);
				ISDataType white[5]={0,0,0,0,0};
				if(STRIP_ALPHA(colourspace)==IS_TYPE_RGB)
					white[0]=white[1]=white[2]=IS_SAMPLEMAX;

				if(factory)
				{
					CMSTransform *transform=factory->GetTransform(target,colourspace);
					if(transform)
						transform->Transform(white,white,1);
				}
				mon->Add(new ImageSource_Solid(colourspace,(pagewidth*res)/72,(pageheight*res)/72,white),0,0);
			}
		}
	}
	catch (const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
	return(result);
}


void Layout_Carousel::DBToLayout(LayoutDB &db)
{
	Layout::DBToLayout(db);

	SetPageExtent(state.printer);
	SetMargins(db.carouseldb.FindInt("LeftMargin"),db.carouseldb.FindInt("RightMargin"),
		db.carouseldb.FindInt("TopMargin"),db.carouseldb.FindInt("BottomMargin"));
	SetSegments(db.carouseldb.FindInt("Segments"));
	SetOverlap(db.carouseldb.FindInt("Overlap"));
	SetAngleOffset(db.carouseldb.FindInt("AngleOffset"));
	SetInnerRadius(db.carouseldb.FindInt("InnerRadius"));
}


void Layout_Carousel::LayoutToDB(LayoutDB &db)
{
	Layout::LayoutToDB(db);

	db.carouseldb.SetInt("LeftMargin",leftmargin);
	db.carouseldb.SetInt("RightMargin",rightmargin);
	db.carouseldb.SetInt("TopMargin",topmargin);
	db.carouseldb.SetInt("BottomMargin",bottommargin);

	db.carouseldb.SetInt("Segments",segments);
	db.carouseldb.SetInt("Overlap",overlap);
	db.carouseldb.SetInt("AngleOffset",angleoffset);
	db.carouseldb.SetInt("InnerRadius",innerradius);
}


GtkWidget *Layout_Carousel::CreateWidget()
{
	return(pp_layout_carousel_new(&state));
}


void Layout_Carousel::RefreshWidget(GtkWidget *widget)
{
	pp_layout_carousel_refresh(PP_LAYOUT_CAROUSEL(widget));
}


const char *Layout_Carousel::GetType()
{
	return("Carousel");
}


void (*Layout_Carousel::SetUnitFunc())(GtkWidget *wid,enum Units unit)
{
	return(pp_layout_carousel_set_unit);
}


int Layout_Carousel::CountImages(int page)
{
	int result=0;
	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		if(ii->page==page)
			++result;
		ii=NextImage();
	}
	return(result);
}


Layout_Carousel_ImageInfo *Layout_Carousel::GetNthImage(int page,int n)
{
	Layout_Carousel_ImageInfo *result=NULL;

	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		if(ii->page==page)
		{
			if(n==0)
				result=(Layout_Carousel_ImageInfo *)ii;
			--n;
		}

		ii=NextImage();
	}
	return(result);
}


void Layout_Carousel::FlushPreview()
{
	if(preview)
		g_object_unref(G_OBJECT(preview));
	preview=NULL;
}


void Layout_Carousel::RenderPreview(int width,int height)
{
	try
	{
		if(imagelist)
		{
			ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_RGBA,72);

			GetImageableArea();

			CircleMontage c(width,height);
			c.SetSegments(segments,angleoffset,overlap);
			int ir=(innerradius*width)/imageablewidth;
			c.SetInnerRadius(ir);

			ImageSource *source=NULL;
			ImageSource *mask=NULL;
			CMSegment *target=NULL;

			for(int s=0;s<segments;s+=2)
			{
				Layout_ImageInfo *img=GetNthImage(currentpage,s);

				if(img)
				{
					GdkPixbuf *thumbnail=img->GetThumbnail();
					source=new ImageSource_GdkPixbuf(thumbnail);
					LayoutRectangle r(source->width,source->height);

					target=c.GetSegmentExtent(s);

					RectFit *fit=r.Fit(*target,true,PP_ROTATION_NONE,img->crop_hpan,img->crop_vpan);

					source=ISScaleImageBySize(source,fit->width,fit->height,IS_SCALING_NEARESTNEIGHBOUR);

					mask=new ImageSource_SegmentMask(target,true);
					source=new ImageSource_Crop(source,fit->xoffset,fit->yoffset,mask->width,mask->height);
	
					source=new ImageSource_Mask(source,mask);
					mon->Add(source,target->x,target->y);

					delete fit;
				}
			}
	
			for(int s=1;s<segments;s+=2)
			{
				Layout_ImageInfo *img=GetNthImage(currentpage,s);

				if(img)
				{
					GdkPixbuf *thumbnail=img->GetThumbnail();
					source=new ImageSource_GdkPixbuf(thumbnail);
					LayoutRectangle r(source->width,source->height);
	
					target=c.GetSegmentExtent(s);
			
					RectFit *fit=r.Fit(*target,true,PP_ROTATION_NONE,img->crop_hpan,img->crop_vpan);
			
					source=ISScaleImageBySize(source,fit->width,fit->height,IS_SCALING_NEARESTNEIGHBOUR);
			
					mask=new ImageSource_SegmentMask(target,false);
					source=new ImageSource_Crop(source,fit->xoffset,fit->yoffset,mask->width,mask->height);
			
					source=new ImageSource_Mask(source,mask);
					mon->Add(source,target->x,target->y);
			
					delete fit;
				}
			}

			preview=pixbuf_from_imagesource(mon,bgcol.red>>8,bgcol.green>>8,bgcol.blue>>8);
			delete mon;
		}
	}
	catch (const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
}


void Layout_Carousel::DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	DrawPreviewBG(widget,xpos,ypos,width,height);

	if(prevwidth!=width || prevheight!=height)
		FlushPreview();
	if(!preview)
		RenderPreview(width,height);

	if(preview)
	{
		gdk_draw_pixbuf(widget->window,NULL,preview,
			0,0,
			xpos,ypos,
			gdk_pixbuf_get_width(preview),
			gdk_pixbuf_get_height(preview),
			GDK_RGB_DITHER_NONE,0,0);
		prevwidth=width;
		prevheight=height;
	}
}


void Layout_Carousel::SetSegments(int segs)
{
	segs=(segs+1)&(~1);
	if(segs<4)
		segs=4;
	if(segments!=segs)
	{
		segments=segs;
		FlushPreview();
	}
}


int Layout_Carousel::GetSegments()
{
	return(segments);
}


void Layout_Carousel::SetOverlap(int ovlp)
{
	if(ovlp<0)
		ovlp=0;
	if(ovlp>50)
		ovlp=50;
	if(overlap!=ovlp)
	{
		overlap=ovlp;
		FlushPreview();
	}
}


int Layout_Carousel::GetOverlap()
{
	return(overlap);
}


void Layout_Carousel::SetAngleOffset(int ao)
{
	angleoffset=ao;
	FlushPreview();
}


int Layout_Carousel::GetAngleOffset()
{
	return(angleoffset);
}


void Layout_Carousel::SetInnerRadius(int ao)
{
	innerradius=ao;
	FlushPreview();
}


int Layout_Carousel::GetInnerRadius()
{
	return(innerradius);
}


int Layout_Carousel::FreeSlots()
{
	int totalslots=segments;
	int count=0;
	int c=g_list_length(imagelist);
	for(int i=0;i<c;++i)
	{
		GList *element=g_list_nth(imagelist,i);
		Layout_Carousel_ImageInfo *ii=(Layout_Carousel_ImageInfo *)element->data;
		if(ii->page==currentpage)
			++count;
	}
	return(totalslots-count);
}


Layout_Carousel_ImageInfo *Layout_Carousel::ImageAt(int page,int segment)
{
	Layout_Carousel_ImageInfo *result=NULL;
	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		Layout_Carousel_ImageInfo *nii=(Layout_Carousel_ImageInfo *)ii;
		if(nii->page==page)
		{
			if(!--segment)
				result=nii;
		}
		ii=NextImage();
	}
	return(result);
}


Layout_ImageInfo *Layout_Carousel::ImageAtCoord(int x,int y)
{
	GetImageableArea();
	CircleMontage c(imageablewidth,imageableheight);
	c.SetSegments(segments,angleoffset,overlap);
	CMSegment *extent=NULL;

	int bestseg=-1;
	int leastdr=100000000;
	for(int seg=0;seg<segments;++seg)
	{
		extent=c.GetSegmentExtent(seg);
		int cx=extent->x+extent->w/2;
		int cy=extent->y+extent->h/2;
		int dr=(cx-x)*(cx-x)+(cy-y)*(cy-y);
		if(dr<leastdr)
		{
			bestseg=seg;
			leastdr=dr;
		}
		delete extent;
	}
	cerr << "Best segment: " << bestseg << endl;
	return(ImageAt(currentpage,bestseg+1));
}


int Layout_Carousel::GetCapabilities()
{
	return(PPLAYOUT_PROFILE|PPLAYOUT_EFFECTS|PPLAYOUT_DUPLICATE);
}
