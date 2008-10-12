/*
 * imagesource_dither.cpp
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

#include "imagesource_dither.h"

using namespace std;

ImageSource_Dither::~ImageSource_Dither()
{
	if(source)
		delete source;
	if(err1)
		free(err1);
	if(err2)
		free(err2);
}


ISDataType *ImageSource_Dither::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	if(row & 1)
	{
		for(int s=0;s<samplesperpixel;++s)
			err2[s]=err2[samplesperpixel+s]=0;

		for(int x=0;x<width;++x)
		{
			for(int s=0;s<samplesperpixel;++s)
			{
				int t=srcdata[x*samplesperpixel+s];
				t+=err1[(x+1)*samplesperpixel+s];
				int u=t&mask;
//				u=(u*IS_SAMPLEMAX)/mask;
				if(t>IS_SAMPLEMAX) u=IS_SAMPLEMAX;
				if(t<0) u=0;
				int err=t-u;
				rowbuffer[x*samplesperpixel+s]=u;
				err1[(x+2)*samplesperpixel+s]+=(7*err)/16;
				err2[(x)*samplesperpixel+s]+=(1*err)/16;
				err2[(x+1)*samplesperpixel+s]+=(5*err)/16;
				err2[(x+2)*samplesperpixel+s]=(3*err)/16;
			}
		}
	}
	else
	{
		for(int s=0;s<samplesperpixel;++s)
			err1[width*samplesperpixel+s]=err1[(width+1)*samplesperpixel+s]=0;

		for(int x=width-1;x>=0;--x)
		{
			for(int s=0;s<samplesperpixel;++s)
			{
				int t=srcdata[x*samplesperpixel+s];
				t+=err2[(x+1)*samplesperpixel+s];
				int u=t&mask;
//				u=(u*IS_SAMPLEMAX)/mask;
				if(t>IS_SAMPLEMAX) u=IS_SAMPLEMAX;
				if(t<0) u=0;
				int err=t-u;
				rowbuffer[x*samplesperpixel+s]=u;
				err2[(x)*samplesperpixel+s]+=(7*err)/16;
				err1[(x)*samplesperpixel+s]=(3*err)/16;
				err1[(x+1)*samplesperpixel+s]+=(5*err)/16;
				err1[(x+2)*samplesperpixel+s]+=(1*err)/16;
			}
		}
	}
	
	currentrow=row;

	return(rowbuffer);
}


ImageSource_Dither::ImageSource_Dither(ImageSource *source,int bitdepth)
	: ImageSource(source), source(source), err1(NULL), err2(NULL)
{
	mask=1;
	while(bitdepth--)
		mask=mask<<1;
	mask-=1;
	cerr << "Mask: " << mask << endl;
	// mask now equals (2^bitdepth)-1 - now left-align it...
	while(mask<(IS_SAMPLEMAX/2))
		mask=mask<<1;
	cerr << "Mask: " << mask << endl;

	err1=(int *)malloc(sizeof(int)*samplesperpixel*(width+2));
	err2=(int *)malloc(sizeof(int)*samplesperpixel*(width+2));
	for(int i=0;i<width+2;++i)
		err1[i]=err2[i]=0;

	MakeRowBuffer();
}
