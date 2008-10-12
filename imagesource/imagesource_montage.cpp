/*
 * imagesource_montage.cpp
 * Composites multiple images into a single image.
 * Supports Random Access if and only if all source images also support it.
 *
 * Copyright (c) 2004-2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * 2005-03-25 - Separated out the n-up image fitting into imagesource_signature.*
 *
 * 2005-03-18 - To reduce memory efficiency, can now flush out components once
 *              the current row has passed them.
 *
 */

// Define this to retain random access capability, at the expense of higher
// memory usage.
#undef MONTAGE_RANDOM_ACCESS


#include <iostream>

#include "imagesource_montage.h"

using namespace std;

class ISMontage_Component
{
	public:
	ISMontage_Component(ImageSource_Montage *header,ImageSource *source,int xpos,int ypos);
	~ISMontage_Component();
	ISDataType *GetRow(int row);
	int	RowDistance(int row);
	protected:
	ImageSource_Montage *header;
	ImageSource *source;
	int xpos,ypos;
	ISMontage_Component *next,*prev;
	friend class ImageSource_Montage;
};


int ISMontage_Component::RowDistance(int row)
{
	// If row is less than the first row of this component,
	// returns the difference, as a negative number.
	// If row is greater than the last row of this component,
	// returns the difference as a positive number.
	// Else, returns zero.
	if(row<ypos)
		return(row-ypos);
	if(row>=(ypos+source->height))
		return(row-(ypos+source->height));
	return(0);
}


ISDataType *ISMontage_Component::GetRow(int row)
{
	if(RowDistance(row)!=0)
		return(NULL);
	else
		return(source->GetRow(row-ypos));
}


ISMontage_Component::ISMontage_Component(ImageSource_Montage *header,ImageSource *src,int xpos,int ypos)
	: header(header), source(src), xpos(xpos), ypos(ypos), next(NULL), prev(NULL)
{
	if((next=header->first))
		next->prev=this;
	header->first=this;
	
	if(header->height<(ypos+source->height))
		header->height=ypos+source->height;

	if(header->width<(xpos+source->width))
		header->width=xpos+source->width;
}


ISMontage_Component::~ISMontage_Component()
{
	if(source)
		delete source;
	if(prev)
		prev->next=next;
	else
		header->first=next;
	if(next)
		next->prev=prev;
}


ImageSource_Montage::ImageSource_Montage(IS_TYPE type,int resolution)
	: ImageSource(), first(NULL)
{
	xres=resolution;
	yres=resolution;
	this->type=type;

	switch(type)
	{
//		case IS_TYPE_BW:
//		case IS_TYPE_GREY:
//			samplesperpixel=1;
//			break;
//		case ISTYPE_GREYA:
//			samplesperpixel=2;
//			break;
		case IS_TYPE_RGB:
			samplesperpixel=3;
			break;
		case IS_TYPE_RGBA:
		case IS_TYPE_CMYK:
			samplesperpixel=4;
			break;
		default:
			throw "Montage: unsupported type";
	}

#ifdef MONTAGE_RANDOM_ACCESS
	randomaccess=true;
#else
	randomaccess=false;
#endif

	width=height=0;
}


ImageSource_Montage::~ImageSource_Montage()
{
	while(first)
		delete first;
}


void ImageSource_Montage::Add(ImageSource *is,int xpos,int ypos)
{
	if(STRIP_ALPHA(is->type)!=STRIP_ALPHA(type))
		throw "Can't yet mix different colour spaces on one page";
	new ISMontage_Component(this,is,xpos,ypos);

	randomaccess&=is->randomaccess;
}


ISDataType *ImageSource_Montage::GetRow(int row)
{
	if(!rowbuffer)
		MakeRowBuffer();

	ISDataType *src;
	ISDataType *dst=rowbuffer;

	switch(type)
	{
		case IS_TYPE_RGBA:
			for(int i=0;i<width*samplesperpixel;i+=samplesperpixel)
			{
				dst[i]=IS_SAMPLEMAX;
				dst[i+1]=IS_SAMPLEMAX;
				dst[i+2]=IS_SAMPLEMAX;
				dst[i+3]=IS_SAMPLEMAX;
			}
			break;
		case IS_TYPE_RGB:
			for(int i=0;i<width*samplesperpixel;++i)
				dst[i]=IS_SAMPLEMAX;
			break;
		default:
			for(int i=0;i<width*samplesperpixel;++i)
				dst[i]=0;
			break;
	}

	ISMontage_Component *mc=first;
	while(mc)
	{
		if((src=mc->GetRow(row)))
		{
			if(HAS_ALPHA(mc->source->type))
			{
				if(HAS_ALPHA(type))
				{
					// If the target image has an alpha channel too, then we
					// must choose a target alpha value.  Perhaps the highest alpha
					// level encountered?
					for(int i=0;i<mc->source->width;++i)
					{
						int a=src[(i+1)*mc->source->samplesperpixel-1];
						int ia=IS_SAMPLEMAX-a;
						int xp=(mc->xpos+i)*samplesperpixel;
						int sp=i*mc->source->samplesperpixel;
						int j;
						for(j=0;j<samplesperpixel-1;++j)
						{
							int t=dst[xp+j];
							t*=ia;
							int t2=src[sp+j];
							t2*=a;
							dst[xp+j]=(t+t2)/IS_SAMPLEMAX;
						}
						if(a>0)
						{
							if(dst[xp+j]>ia)
								dst[xp+j]=ia;
//							dst[xp+j]=ia;
						}
//						if(dst[xp+j]>a)
//							dst[xp+j]=0;
//							dst[xp+j]=0;
					}
				}
				else
				{
					// The source image has an alpha channel, but is being composited
					// onto an image without.
					for(int i=0;i<mc->source->width;++i)
					{
						int a=src[(i+1)*mc->source->samplesperpixel-1];
//						cerr << "Alpha: " << a << endl;
						int ia=IS_SAMPLEMAX-a;
						int xp=(mc->xpos+i)*samplesperpixel;
						int sp=i*mc->source->samplesperpixel;
						for(int j=0;j<samplesperpixel;++j)
						{
							int t=dst[xp+j];
							t*=ia;
							int t2=src[sp+j];
							t2*=a;
							dst[xp+j]=(t+t2)/IS_SAMPLEMAX;
						}
					}
				}
			}
			else
			{
				// Source component has no alpha channel, but the target image does.
				if(HAS_ALPHA(type))
				{
					for(int i=0;i<mc->source->width;++i)
					{
						int j;
						for(j=0;j<samplesperpixel-1;++j)
						{
							dst[(mc->xpos+i)*samplesperpixel+j]=src[i*mc->source->samplesperpixel+j];
						}
						dst[(mc->xpos+i)*samplesperpixel+j]=IS_SAMPLEMAX;
					}
				}
				else
				{
					// Simplest case - neither source nor destination has an alpha channel.
					for(int i=0;i<mc->source->width*samplesperpixel;++i)
						dst[mc->xpos*samplesperpixel+i]=src[i];
				}
			}
		}
		ISMontage_Component *nmc=mc->next;
#ifndef MONTAGE_RANDOM_ACCESS
		if(mc->RowDistance(row)>0)
			delete mc;
#endif
		mc=nmc;
	}

	return(rowbuffer);
}
