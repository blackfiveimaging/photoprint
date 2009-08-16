/*
 * layout_nup.cpp - Has responsibility for tracking the list of images and their layout.
 * Also has responsibility for building the ImageSource stack at print time.
 * Subclass of Signature, which provides coordinates of image rectangles in n-up layout.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2004-12-10: No longer opens the imagesource when placing an image, since we should now
 *             be able to handle anything GdkPixbuf can handle.
 *
 * 2004-12-24  Getting the actual imagesource (with CMS transform) from the ImageInfo
 *             is now delegated to the superclasses.
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
#include "imagesource/imagesource_scale.h"
#include "imagesource/imagesource_rotate.h"
#include "imagesource/imagesource_crop.h"
#include "imagesource/imagesource_promote.h"
#include "imagesource/imagesource_montage.h"
#include "imagesource/imagesource_solid.h"

#include "photoprint_state.h"
#include "pp_layout_nup.h"

#include "layout_nup.h"

using namespace std;


ConfigTemplate Layout_NUpDB::Template[]=
{
	ConfigTemplate("Rows",int(2)),
	ConfigTemplate("Columns",int(2)),
	ConfigTemplate("HGutter",int(DEFAULTGUTTER)),
	ConfigTemplate("VGutter",int(DEFAULTGUTTER)),
	ConfigTemplate("LeftMargin",int(-1)),
	ConfigTemplate("RightMargin",int(-1)),
	ConfigTemplate("TopMargin",int(-1)),
	ConfigTemplate("BottomMargin",int(-1)),
	ConfigTemplate()
};


Layout_NUp_ImageInfo::Layout_NUp_ImageInfo(Layout_NUp &layout, const char *filename,int row,int column,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,filename,page,allowcropping,rotation),
	row(row), column(column)
{
}


Layout_NUp_ImageInfo::Layout_NUp_ImageInfo(Layout_NUp &layout, Layout_ImageInfo *ii,int row,int column,int page,bool allowcropping,PP_ROTATION rotation)
	: Layout_ImageInfo((Layout &)layout,ii,page,allowcropping,rotation),
	row(row), column(column)
{
}


Layout_NUp_ImageInfo::~Layout_NUp_ImageInfo()
{
}


LayoutRectangle *Layout_NUp_ImageInfo::GetBounds()
{
	Layout_NUp *l=(Layout_NUp *)&layout;
	return(l->GetLayoutRectangle(row,column));
}

void Layout_NUp_ImageInfo::DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height)
{
	LayoutRectangle *target=GetBounds();
	double scale=width;
	scale/=layout.pagewidth;
	target->Scale(scale);
	Layout_ImageInfo::DrawThumbnail(widget,xpos+target->x,ypos+target->y,target->w,target->h);
	delete target;
}


Layout_NUp::Layout_NUp(PhotoPrint_State &state,Layout *oldlayout)
	: Layout(state,oldlayout), Signature()
{
}


Layout_NUp::~Layout_NUp()
{
}


void Layout_NUp::Reflow()
{
	int row=0;
	int column=-1;
	int page=0;

	CancelRenderThreads();

	for(unsigned int i=0;i<g_list_length(imagelist);++i)
	{
		++column;
		if(column>=GetColumns())
		{
			column=0;
			++row;
			if(row>=GetRows())
			{
				row=0;
				++page;
			}
		}		
		GList *element=g_list_nth(imagelist,i);
		Layout_NUp_ImageInfo *ii=(Layout_NUp_ImageInfo *)element->data;
		ii->row=row;
		ii->column=column;
		ii->page=page;
	}
	pages=page+1;
	if(currentpage>page)
		currentpage=page;
}


bool Layout_NUp::PlaceImage(const char *filename,int page,int row, int column,bool cropfit,PP_ROTATION rotate)
{
	Layout_NUp_ImageInfo *ii=NULL;
	try
	{
		ii=new Layout_NUp_ImageInfo(*this,filename,row,column,page,cropfit,rotate);
	}
	catch(const char *msg)
	{
		cerr << "Caught exception" << endl;
		ErrorMessage_Dialog(msg);
		if(ii)
			delete ii;
	}
	if(ii)
	{
		imagelist=g_list_append(imagelist,ii);
		
		if(page>=pages)
			++pages;
		cerr << "Bumped page numbers" << endl;
		return(true);
	}
	return(false);
}


int Layout_NUp::FreeSlots()
{
	int totalslots=GetRows()*GetColumns();
	int count=0;
	int c=g_list_length(imagelist);
	for(int i=0;i<c;++i)
	{
		GList *element=g_list_nth(imagelist,i);
		Layout_NUp_ImageInfo *ii=(Layout_NUp_ImageInfo *)element->data;
		if(ii->page==currentpage)
			++count;
	}
	return(totalslots-count);
}


void Layout_NUp::FindFirstFree(int &page,int &row,int &column)
{
	page=0;
	row=0;
	column=-1;
	int count=g_list_length(imagelist);
	for(int i=0;i<count;++i)
	{
		GList *element=g_list_nth(imagelist,i);
		Layout_NUp_ImageInfo *ii=(Layout_NUp_ImageInfo *)element->data;
		if(ii->page>=page)
		{
			if(ii->page>page)
			{
				row=0;
				column=-1;
			}
			page=ii->page;
			if(ii->row>=row)
			{
				if(ii->row>row)
					column=-1;
				row=ii->row;
				if(ii->column>column)
					column=ii->column;				
			}
		}
	}
	++column;
	if(column>=GetColumns())
	{
		column=0;
		++row;
		if(row>=GetRows())
		{
			row=0;
			++page;
		}
	}
}


int Layout_NUp::AddImage(const char *filename,bool allowcropping,PP_ROTATION rotation)
{
	int page,row,column;
	FindFirstFree(page,row,column);
	cerr << "Placing image at " << page << ", " << row << ", " << column << endl;
	if(PlaceImage(filename,page,row,column,allowcropping,rotation))
		return(page);
	else
		return(currentpage);
}


void Layout_NUp::CopyImage(Layout_ImageInfo *ii)
{
	int page,row,column;
	FindFirstFree(page,row,column);
	ii=new Layout_NUp_ImageInfo(*this,ii,row,column,page,ii->allowcropping,ii->rotation);
	imagelist=g_list_append(imagelist,ii);
	if(page>=pages)
		++pages;
}


ImageSource *Layout_NUp::GetImageSource(int page,CMColourDevice target,CMTransformFactory *factory,int res,bool completepage)
{
	ImageSource *result=NULL;
	if(imagelist)
	{
		enum IS_TYPE colourspace=GetColourSpace(target);

		IS_ScalingQuality qual=IS_ScalingQuality(state.FindInt("ScalingQuality"));
		if(!res)
			res=state.FindInt("RenderingResolution");

		ImageSource_Montage *mon=new ImageSource_Montage(colourspace,res);

		Layout_NUp_ImageInfo *ii=(Layout_NUp_ImageInfo *)FirstImage();
		while(ii)
		{
			if(ii->page==page)
			{
				ImageSource *img=ii->GetImageSource(target,factory);
				if(img)
				{
					LayoutRectangle r(img->width,img->height);
					LayoutRectangle *target=ii->GetBounds();
					double scale=res;
					scale/=72.0;
					target->Scale(scale);
					
					RectFit *fit=r.Fit(*target,ii->allowcropping,ii->rotation,ii->crop_hpan,ii->crop_vpan);

					if(fit->rotation)
						img=new ImageSource_Rotate(img,fit->rotation);
					
					img=ISScaleImageBySize(img,fit->width,fit->height,qual);
					img->SetResolution(res,res);
					
					if(fit->width>target->w)
						fit->width=target->w;
					if(fit->height>target->h)
						fit->height=target->h;

					if(img->width<fit->width)
						fit->width=img->width;
					if(img->height<fit->height)
						fit->height=img->height;

					cerr << "xoffset: " << fit->xoffset << endl;
					cerr << "yoffset: " << fit->yoffset << endl;

					if(ii->allowcropping)
					{
						cerr << "Cropping" << endl;
						img=new ImageSource_Crop(img,fit->xoffset,fit->yoffset,fit->width,fit->height);
					}
					else
						cerr << "Not cropping" << endl;

					img=ii->ApplyMask(img);

					mon->Add(img,fit->xpos,fit->ypos);
					delete fit;
				}
			}
			ii=(Layout_NUp_ImageInfo *)NextImage();
		}
		result=mon;

		// Load background image, if present...
		if(backgroundfilename)
		{
			ImageSource *is=ISLoadImage(backgroundfilename);

			IS_TYPE colourspace=GetColourSpace(target);

			if(STRIP_ALPHA(is->type)==IS_TYPE_GREY)
				is=new ImageSource_Promote(is,colourspace);

			if(STRIP_ALPHA(is->type)==IS_TYPE_BW)
				is=new ImageSource_Promote(is,colourspace);

			if(factory)
			{
				CMSTransform *transform=factory->GetTransform(target,is);
				if(transform)
					is=new ImageSource_CMS(is,transform);
			}
			if((is->width<is->height)^(pagewidth<pageheight))
				is=new ImageSource_Rotate(is,90);
			is=ISScaleImageBySize(is,(pagewidth*res)/72,(pageheight*res)/72,qual);
			mon->Add(is,0,0);
		}
		else if(completepage)
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
	return(result);
}


void Layout_NUp::DBToLayout(LayoutDB &db)
{
	Layout::DBToLayout(db);

	SetPageExtent(state.printer);
	SetRows(db.nupdb.FindInt("Rows"));
	SetColumns(db.nupdb.FindInt("Columns"));
	SetGutters(db.nupdb.FindInt("HGutter"),db.nupdb.FindInt("VGutter"));
	SetMargins(db.nupdb.FindInt("LeftMargin"),db.nupdb.FindInt("RightMargin"),
		db.nupdb.FindInt("TopMargin"),db.nupdb.FindInt("BottomMargin"));
}


void Layout_NUp::LayoutToDB(LayoutDB &db)
{
	Layout::LayoutToDB(db);

	db.nupdb.SetInt("Rows",GetRows());
	db.nupdb.SetInt("Columns",GetColumns());
	db.nupdb.SetInt("HGutter",GetHGutter());
	db.nupdb.SetInt("VGutter",GetVGutter());
	db.nupdb.SetInt("LeftMargin",leftmargin);
	db.nupdb.SetInt("RightMargin",rightmargin);
	db.nupdb.SetInt("TopMargin",topmargin);
	db.nupdb.SetInt("BottomMargin",bottommargin);
}


Layout_NUp_ImageInfo *Layout_NUp::ImageAt(int page, int row, int column)
{
	Layout_NUp_ImageInfo *result=NULL;
	Layout_ImageInfo *ii=FirstImage();
	while(ii)
	{
		Layout_NUp_ImageInfo *nii=(Layout_NUp_ImageInfo *)ii;
		if(nii->page==page && nii->row==row && nii->column==column)
		{
			result=nii;
		}
		ii=NextImage();
	}
	return(result);
}


GtkWidget *Layout_NUp::CreateWidget()
{
	return(pp_layout_nup_new(&state));
}


void Layout_NUp::RefreshWidget(GtkWidget *widget)
{
	pp_layout_nup_refresh(PP_LAYOUT_NUP(widget));
}


const char *Layout_NUp::GetType()
{
	return("NUp");
}


void (*Layout_NUp::SetUnitFunc())(GtkWidget *wid,enum Units unit)
{
	return(pp_layout_nup_set_unit);
}


Layout_ImageInfo *Layout_NUp::ImageAtCoord(int x,int y)
{
	int r=RowAt(y);
	int c=ColumnAt(x);
	return(ImageAt(GetCurrentPage(),r,c));
}


int Layout_NUp::GetCapabilities()
{
	return(PPLAYOUT_CROP|PPLAYOUT_ROTATE|PPLAYOUT_MASK|PPLAYOUT_EFFECTS|PPLAYOUT_BACKGROUND|PPLAYOUT_PROFILE|PPLAYOUT_DUPLICATE);
}

