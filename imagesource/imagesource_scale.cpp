/*
 * imagesource_scale.cpp - nearest-neighbour scaling filter
 *
 * Supports Greyscale, RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_scale.h"

using namespace std;


// Scaling implemented as a chaining of horizontal and vertical scaling

ImageSource_Scale::~ImageSource_Scale()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Scale::GetRow(int row)
{
	return(source->GetRow(row));
}



ImageSource_Scale::ImageSource_Scale(struct ImageSource *source,int width, int height)
	: ImageSource(source), source(source)
{
	this->source=new ImageSource_HScale(this->source,width);
	this->source=new ImageSource_VScale(this->source,height);
	xres=this->source->xres;
	yres=this->source->yres;
	MakeRowBuffer();
}



// Vertical scaling


ImageSource_VScale::~ImageSource_VScale()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_VScale::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	int srcrow=(row*source->height)/height;
	ISDataType *srcdata=source->GetRow(srcrow);

	for(i=0;i<width*samplesperpixel;++i)
	{
		rowbuffer[i]=srcdata[i];
	}

	currentrow=row;

	return(rowbuffer);
}



ImageSource_VScale::ImageSource_VScale(struct ImageSource *source,int height)
	: ImageSource(source), source(source)
{
	this->height=height;
	yres=height*source->yres; yres/=source->height;
	MakeRowBuffer();
}


// Horizontal Scaling


ImageSource_HScale::~ImageSource_HScale()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_HScale::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	switch(source->samplesperpixel)
	{
		case 1:
			for(i=0;i<width;++i)
			{
				int sx=(i*source->width)/width;
				rowbuffer[i]=srcdata[sx];
			}
			break;
		case 3:
			for(i=0;i<width;++i)
			{
				int sx=(i*source->width)/width;
				rowbuffer[i*3]=srcdata[sx*3];
				rowbuffer[i*3+1]=srcdata[sx*3+1];
				rowbuffer[i*3+2]=srcdata[sx*3+2];
			}
			break;
		default:
			for(i=0;i<width;++i)
			{
				int sx=(i*source->width)/width;
				for(int j=0;j<samplesperpixel;++j)
					rowbuffer[i*samplesperpixel+j]=srcdata[sx*samplesperpixel+j];
			}
			break;
	}

	currentrow=row;

	return(rowbuffer);
}



ImageSource_HScale::ImageSource_HScale(struct ImageSource *source,int width)
	: ImageSource(source), source(source)
{
	this->width=width;
	xres=width*source->xres; xres/=source->width;
	MakeRowBuffer();
}
