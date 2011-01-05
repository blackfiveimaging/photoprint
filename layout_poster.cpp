/*
 * layout_poster.cpp - Has responsibility for tracking the list of images and their layout.
 * Also has responsibility for building the ImageSource stack at print time.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */

#include <iostream>
#include <string.h>

#include "support/debug.h"
#include "support/layoutrectangle.h"

#include "miscwidgets/generaldialogs.h"
#include "dialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "imageutils/rotatepixbuf.h"

#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_crop.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_util.h"
#include "imagesource/imagesource_flatten.h"
#include "imagesource/imagesource_montage.h"
#include "imagesource/imagesource_solid.h"

#include "photoprint_state.h"
#include "pp_layout_poster.h"
#include "pp_layout_poster_pageview.h"

#include "layout_poster.h"

using namespace std;


ConfigTemplate Layout_PosterDB::Template[]=
{
	ConfigTemplate("LeftMargin",int(0)),
	ConfigTemplate("RightMargin",int(0)),
	ConfigTemplate("TopMargin",int(0)),
	ConfigTemplate("BottomMargin",int(0)),
	ConfigTemplate("HorizontalOverlap",int(8)),
	ConfigTemplate("VerticalOverlap",int(8)),
	ConfigTemplate("PosterWidth",int(1000)),
	ConfigTemplate("PosterHeight",int(1000)),
	ConfigTemplate()
};


Layout_Poster_ImageInfo::Layout_Poster_ImageInfo(Layout_Poster &layout, const char *filename,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,filename,page,allowcropping,rotation)
{
}


Layout_Poster_ImageInfo::Layout_Poster_ImageInfo(Layout_Poster &layout, Layout_ImageInfo *ii,int page)
	: Layout_ImageInfo((Layout &)layout,ii,page)
{
}


Layout_Poster_ImageInfo::~Layout_Poster_ImageInfo()
{
}


void Layout_Poster_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	Layout_Poster *l=(Layout_Poster *)&layout;	
	LayoutRectangle target(l->leftmargin,l->topmargin,l->posterwidth,l->posterheight);
	double scale=width;
	scale/=l->paperwidth;
	target.Scale(scale);
	Layout_ImageInfo::DrawThumbnail(widget,xpos+target.x,ypos+target.y,target.w,target.h);
}


LayoutRectangle *Layout_Poster_ImageInfo::GetBounds()
{
	Layout_Poster *l=(Layout_Poster *)&layout;	
	return(new LayoutRectangle(l->leftmargin,l->topmargin,l->posterwidth,l->posterheight));
}


Layout_Poster::Layout_Poster(PhotoPrint_State &state,Layout *oldlayout)
	: Layout(state,oldlayout), posters(1), currentposter(0), htiles(1), vtiles(1)
{
}


Layout_Poster::~Layout_Poster()
{
}


void Layout_Poster::Reflow()
{
	int page=0;
	LayoutIterator it(*this);
	Layout_Poster_ImageInfo *ii=(Layout_Poster_ImageInfo *)it.FirstImage();
	while(ii)
	{
		ii->page=page;
		ii=(Layout_Poster_ImageInfo *)it.NextImage();
		posters=page;
		++page;
	}
	SetCurrentPage(1);
	pages=page*htiles*vtiles;
}


int Layout_Poster::AddImage(const char *filename,bool allowcropping,PP_ROTATION rotation)
{
	int page=posters+1;
	for(int i=0;i<=posters;++i)
	{
		if(!ImageAt(i))
		{
			page=i;
			i=posters;
		}
	}
	Layout_Poster_ImageInfo *ii=NULL;
	try
	{
		ii=new Layout_Poster_ImageInfo(*this,filename,page,allowcropping,rotation);
	}
	catch(const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
	if(ii)
	{
		imagelist.push_back(ii);

		if(page>=posters)
			++posters;
		pages=posters*htiles*vtiles;
		return(page*htiles*vtiles);
	}
	return(currentpage);
}


void Layout_Poster::CopyImage(Layout_ImageInfo *ii)
{
	int page=posters+1;
	for(int i=0;i<=posters;++i)
	{
		if(!ImageAt(i))
		{
			page=i;
			i=posters;
		}
	}
	ii=new Layout_Poster_ImageInfo(*this,ii,page);
	imagelist.push_back(ii);
	if(page>=posters)
		++posters;
	pages=posters*htiles*vtiles;
}


// FIXME - this routine is *UGLY*.

ImageSource *Layout_Poster::GetImageSource(int page,CMColourDevice target,CMTransformFactory *factory,int res,bool completepage)
{
	ImageSource *is=NULL;
	try
	{
		int p=page/(htiles*vtiles);
		int r=page-htiles*vtiles*p;
		int vt=r/htiles;
		int ht=r-(htiles*vt);

		Debug[TRACE] << "HT: " << ht << endl;
		Debug[TRACE] << "VT: " << vt << endl;

		Layout_Poster_ImageInfo *ii=(Layout_Poster_ImageInfo *)ImageAt(p);
		
		if(ii)
		{
			GetImageableArea();

			is=ii->GetImageSource(target,factory);
			LayoutRectangle srcr(is->width,is->height);
			LayoutRectangle target(posterwidth,posterheight);
			
			RectFit *fit=srcr.Fit(target,ii->allowcropping,ii->rotation,ii->crop_hpan,ii->crop_vpan);

			if(fit->rotation)
				is=new ImageSource_Rotate(is,fit->rotation);

			int l=ht*(imageablewidth-hoverlap);
			int r=(ht+1)*imageablewidth-ht*hoverlap;
			int t=vt*(imageableheight-voverlap);
			int b=(vt+1)*imageableheight-vt*voverlap;

			Debug[TRACE] << "Left: " << l << ", Right: " << r << endl;
			Debug[TRACE] << "Top: " << t << ", Bottom: " << b << endl;

			xoffset=leftmargin;
			yoffset=topmargin;

			l-=fit->xpos;
			r-=fit->xpos;
			t-=fit->ypos;
			b-=fit->ypos;

			Debug[TRACE] << "Left: " << l << ", Right: " << r << endl;
			Debug[TRACE] << "Top: " << t << ", Bottom: " << b << endl;

			if(l<0)
			{
				l=0;
				xoffset+=fit->xpos;
			}
			
			if(t<0)
			{
				t=0;
				yoffset+=fit->ypos;
			}
			
			if(r>fit->width) r=fit->width;
			if(b>fit->height) b=fit->height;

			l+=fit->xoffset;
			r+=fit->xoffset;
			t+=fit->yoffset;
			b+=fit->yoffset;
			
			Debug[TRACE] << "Left: " << l << ", Right: " << r << endl;
			Debug[TRACE] << "Top: " << t << ", Bottom: " << b << endl;

			l=(is->width*l)/fit->width;
			r=(is->width*r)/fit->width;
			t=(is->height*t)/fit->height;
			b=(is->height*b)/fit->height;

			Debug[TRACE] << "Left: " << l << ", Right: " << r << endl;
			Debug[TRACE] << "Top: " << t << ", Bottom: " << b << endl;

			is=ii->ApplyMask(is);
			is=new ImageSource_Flatten(is);

			Debug[TRACE] << "Old resolution: " << is->xres << " x " << is->yres << " dpi" << endl;
			is->SetResolution(72.0/fit->scale,72.0/fit->scale);

			is=new ImageSource_Crop(is,l,t,r-l,b-t);


			IS_ScalingQuality qual=IS_ScalingQuality(state.FindInt("ScalingQuality"));
			if(!res)
				res=state.FindInt("RenderingResolution");

			is=ISScaleImageByResolution(is,res,res,qual);

			delete fit;
		}
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
				RefCountPtr<CMSTransform> transform;
				transform=factory->GetTransform(target,colourspace);
				if(transform)
					transform->Transform(white,white,1);
			}

			ImageSource_Montage *mon=new ImageSource_Montage(colourspace,res);
			mon->Add(is,(xoffset*res)/72,(yoffset*res)/72);
			mon->Add(new ImageSource_Solid(colourspace,(pagewidth*res)/72,(pageheight*res)/72,white),0,0);
			is=mon;
		}
	}
	catch (const char *msg)
	{
		ErrorMessage_Dialog(msg);
	}
	return(is);
}


void Layout_Poster::SetPageExtent(PageExtent &pe)
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


void Layout_Poster::SetMargins(int left,int right,int top,int bottom)
{
	if((left+right)>=pagewidth)
		Debug[WARN] << "Margins are too wide!" << endl;
	else
	{
		leftmargin=left;
		rightmargin=right;
	}
	if((top+bottom)>=pageheight)
		Debug[WARN] << "Margins are too tall!" << endl;
	else
	{
		topmargin=top;
		bottommargin=bottom;
	}
}


void Layout_Poster::TilesFromSize()
{
	GetImageableArea();
	htiles=int(double(posterwidth-hoverlap)/(imageablewidth-hoverlap)+0.999);
	vtiles=int(double(posterheight-voverlap)/(imageableheight-voverlap)+0.999);
	paperwidth = leftmargin + rightmargin + imageablewidth*htiles - (htiles-1)*hoverlap;
	paperheight = topmargin + bottommargin + imageableheight*vtiles - (vtiles-1)*voverlap;
	pages=posters*htiles*vtiles;
	Layout::SetCurrentPage(currentposter*htiles*vtiles);
}


void Layout_Poster::SizeFromTiles()
{
	GetImageableArea();
	posterwidth = imageablewidth*htiles - (htiles-1)*hoverlap;
	posterheight = imageableheight*vtiles - (vtiles-1)*voverlap;
	paperwidth=leftmargin + rightmargin + posterwidth;
	paperheight=topmargin + bottommargin + posterheight;
	pages=posters*htiles*vtiles;
	Layout::SetCurrentPage(currentposter*htiles*vtiles);
}


void Layout_Poster::DBToLayout(LayoutDB &db)
{
	Layout::DBToLayout(db);

	SetPageExtent(state.printer);
	SetMargins(db.posterdb.FindInt("LeftMargin"),db.posterdb.FindInt("RightMargin"),
		db.posterdb.FindInt("TopMargin"),db.posterdb.FindInt("BottomMargin"));
	hoverlap=db.posterdb.FindInt("HorizontalOverlap");
	voverlap=db.posterdb.FindInt("VerticalOverlap");
	posterwidth=db.posterdb.FindInt("PosterWidth");
	posterheight=db.posterdb.FindInt("PosterHeight");
	TilesFromSize();
}


void Layout_Poster::LayoutToDB(LayoutDB &db)
{
	Layout::LayoutToDB(db);

	db.posterdb.SetInt("LeftMargin",leftmargin);
	db.posterdb.SetInt("RightMargin",rightmargin);
	db.posterdb.SetInt("TopMargin",topmargin);
	db.posterdb.SetInt("BottomMargin",bottommargin);
	db.posterdb.SetInt("HorizontalOverlap",hoverlap);
	db.posterdb.SetInt("VerticalOverlap",voverlap);
	db.posterdb.SetInt("PosterWidth",posterwidth);
	db.posterdb.SetInt("PosterHeight",posterheight);
}


Layout_Poster_ImageInfo *Layout_Poster::ImageAt(int page)
{
	Layout_Poster_ImageInfo *result=NULL;
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		Layout_Poster_ImageInfo *nii=(Layout_Poster_ImageInfo *)ii;
		if(nii->page==page)
		{
			result=nii;
		}
		ii=it.NextImage();
	}
	return(result);
}


GtkWidget *Layout_Poster::CreateWidget()
{
	return(pp_layout_poster_new(&state));
}


void Layout_Poster::RefreshWidget(GtkWidget *widget)
{
	pp_layout_poster_refresh(PP_LAYOUT_POSTER(widget));
}


void Layout_Poster::DrawGridLines(GtkWidget *widget)
{
	pp_layout_poster_pageview_draw_gridlines(PP_LAYOUT_POSTER_PAGEVIEW(widget));
}


const char *Layout_Poster::GetType()
{
	return("Poster");
}


void Layout_Poster::SetCurrentPage(int page)
{
	Layout::SetCurrentPage(page);
	currentposter=page/(htiles*vtiles);
}


void (*Layout_Poster::SetUnitFunc())(GtkWidget *wid,enum Units unit)
{
	return(pp_layout_poster_set_unit);
}


void Layout_Poster::DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	DrawPreviewBG(widget,xpos,ypos,width,height);

	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	int cp=currentpage/(htiles*vtiles);

	while(ii)
	{
		if(cp==ii->page)
		{
			ii->DrawThumbnail(widget,xpos,ypos,width,height);
		}
		ii=it.NextImage();
	}
}


int Layout_Poster::GetCapabilities()
{
	return(PPLAYOUT_CROP|PPLAYOUT_ROTATE|PPLAYOUT_MASK|PPLAYOUT_EFFECTS|PPLAYOUT_PROFILE);
}


bool Layout_Poster_ImageInfo::GetSelected()
{
	Layout_Poster *l=(Layout_Poster *)&layout;
	return(page==l->currentposter);
}

#if 0
Layout_ImageInfo *Layout_Poster::FirstSelected()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.FirstImage();
	while(ii)
	{
		if(ii->page==currentposter)
			return(ii);
		ii=it.NextSelected();
	}
	return(NULL);
}


Layout_ImageInfo *Layout_Poster::NextSelected()
{
	LayoutIterator it(*this);
	Layout_ImageInfo *ii=it.NextImage();
	while(ii)
	{
		if(ii->page==currentposter)
			return(ii);
		ii=it.NextSelected();
	}
	return(NULL);
}
#endif

