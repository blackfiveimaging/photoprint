/*
 * imagesource_convolution.cpp - Applies a convolution filter to an image.
 *
 * Supports Greyscale, RGB and CMYK data
 * Doesn't (yet) support random access
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_convolution.h"

using namespace std;

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

// The row cache is just a simplistic ring-buffer type cache which handles
// the details of tracking several rows of "support" data.

class ISConvolution_RowCache
{
	public:
	ISConvolution_RowCache(ImageSource_Convolution *source);
	~ISConvolution_RowCache();
	float *GetRow(int row);
	private:
	ImageSource_Convolution *source;
	float *cache;
	int cachewidth,cachehoffset;
	int bufferrows;
	int currentrow;
};


ISConvolution_RowCache::~ISConvolution_RowCache()
{
	if(cache)
		free(cache);
}


ISConvolution_RowCache::ISConvolution_RowCache(ImageSource_Convolution *source)
	: source(source), currentrow(-1)
{
	cachewidth=source->width+source->hextra*2;
	cachehoffset=source->hextra;
	bufferrows=source->vextra*2+1;
	cache=(float *)malloc(sizeof(float)*source->samplesperpixel*cachewidth*bufferrows);
}


inline float *ISConvolution_RowCache::GetRow(int row)
{
	if(row<0)
		row=0;
	if(row>=source->source->height)
		row=source->source->height-1;
	int crow=row%(source->vextra*2+1);
	float *rowptr=cache+crow*source->samplesperpixel*cachewidth;
	if(row>currentrow)
	{
		currentrow=row;
		ISDataType *src=source->source->GetRow(row);
		for(int x=0;x<cachewidth;++x)
		{
			int sx=x-cachehoffset;
			if(sx<0) sx=0;
			if(sx>=source->width) sx=source->width-1;
			for(int s=0;s<source->samplesperpixel;++s)
			{
				float a=src[sx*source->samplesperpixel+s];
				rowptr[x*source->samplesperpixel+s]=a;
			}
		}
	}
	return(rowptr+cachehoffset*source->samplesperpixel);		
}


ImageSource_Convolution::~ImageSource_Convolution()
{
	if(tmprows)
		free(tmprows);
	if(cache)
		delete cache;
	if(source)
		delete source;
}


ISDataType *ImageSource_Convolution::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	int kw=kernel->GetWidth();
	int kh=kernel->GetHeight();

	for(int r=0;r<kh;++r)
	{
		tmprows[r]=cache->GetRow(row+(r-vextra));
	}

	for(int x=0;x<width;++x)
	{
		float t[5]={0.0,0.0,0.0,0.0,0.0};
		for(int ky=0;ky<kh;++ky)
		{
			for(int kx=0;kx<kw;++kx)
			{
				for(int s=0;s<samplesperpixel;++s)
				{
					t[s]+=kernel->Kernel(kx,ky) * tmprows[ky][(x+(kx-hextra))*samplesperpixel+s];
				}
			}
		}
		for(int s=0;s<samplesperpixel;++s)
		{
			float a=t[s];
			if(a<0.0) a=0.0;
			if(a>IS_SAMPLEMAX) a=IS_SAMPLEMAX;
			rowbuffer[x*samplesperpixel+s]=ISDataType(a);
		}
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_Convolution::ImageSource_Convolution(struct ImageSource *source,ConvKernel *kernel)
	: ImageSource(source), source(source), kernel(kernel)
{
	hextra=kernel->GetWidth()/2;
	vextra=kernel->GetHeight()/2;
	cache=new ISConvolution_RowCache(this);
	tmprows=(float **)malloc(sizeof(float *)*kernel->GetHeight());
	MakeRowBuffer();
	randomaccess=false;
}
