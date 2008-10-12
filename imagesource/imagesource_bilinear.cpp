/*
 * imagesource_bilinear.cpp - Interpolated scaling filter
 * Implements bilinear scaling
 *
 * Supports Greyscale, RGB and CMYK data
 * Doesn't (yet) support random access
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

#include "imagesource_bilinear.h"

using namespace std;


ImageSource_Bilinear::~ImageSource_Bilinear()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Bilinear::GetRow(int row)
{
	return(source->GetRow(row));
}


ImageSource_Bilinear::ImageSource_Bilinear(struct ImageSource *source,int width,int height)
	: ImageSource(source), source(source)
{
	xres=(source->xres*width); xres/=source->width;
	yres=(source->yres*height); yres/=source->height;
	this->source=new ImageSource_HBilinear(this->source,width);
	this->source=new ImageSource_VBilinear(this->source,height);
	randomaccess=this->source->randomaccess;
	this->width=width;
	this->height=height;
}



// Horizontal scaling only


ImageSource_HBilinear::~ImageSource_HBilinear()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_HBilinear::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	ISDataType *src=source->GetRow(row);

	for(i=0;i<width;++i)
	{
		int x1=(i*source->width)/width;
		int x2=x1+1;
		if(x2 >= source->width)
			x2=x1;
		float xfactor=(i*source->width);
		xfactor/=width;
		xfactor-=x1;

		x1*=samplesperpixel;
		x2*=samplesperpixel;

		for(int s=0;s<samplesperpixel;++s)
		{
			double s1=src[x1+s];
			double s2=src[x2+s];

			double t=(1.0-xfactor)*s1+xfactor*s2;

			rowbuffer[i*samplesperpixel+s]=int(t);
		}
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_HBilinear::ImageSource_HBilinear(struct ImageSource *source,int width)
	: ImageSource(source), source(source)
{
	this->width=width;
	xres=(source->xres*width); xres/=source->width;

	MakeRowBuffer();
}


// Vertical scaling only


ImageSource_VBilinear::~ImageSource_VBilinear()
{
	if(source)
		delete source;

	if(lastrow)
		free(lastrow);
}


ISDataType *ImageSource_VBilinear::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	int srow1=(row*source->height)/height;
	int srow2=srow1+1;
	if(srow2>=source->height)
		srow2=srow1;

	ISDataType *src1,*src2;

	if(srow1==cachedrow)
	{
		src1=lastrow;
	}
	else
	{
		src1=source->GetRow(srow1);
		for(int i=0;i<source->width*source->samplesperpixel;++i)
			lastrow[i]=src1[i];
		cachedrow=srow1;
		src1=lastrow;
	}

	src2=source->GetRow(srow2);

	double yfactor=row*source->height;
	yfactor/=height;
	yfactor-=srow1;

	for(i=0;i<width*samplesperpixel;++i)
	{
		double s1=src1[i];
		double s3=src2[i];

		double t=(1.0-yfactor)*s1+yfactor*s3;

		rowbuffer[i]=int(t);
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_VBilinear::ImageSource_VBilinear(struct ImageSource *source,int height)
	: ImageSource(source), source(source), lastrow(NULL), cachedrow(-1)
{
	this->height=height;
	yres=(source->yres*height); yres/=source->height;

	lastrow=(ISDataType *)malloc(width*samplesperpixel*sizeof(ISDataType));
	MakeRowBuffer();
	randomaccess=false;
}


