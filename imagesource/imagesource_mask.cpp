/*
 * imagesource_mask.cpp
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

#include "imagesource_mask.h"

using namespace std;

ImageSource_Mask::~ImageSource_Mask()
{
	if(source)
		delete source;

	if(mask)
		delete mask;
}


ISDataType *ImageSource_Mask::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);
	ISDataType *maskdata=mask->GetRow(row);

	if(source->type&IS_TYPE_ALPHA)
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel-1;++s)
				rowbuffer[x*samplesperpixel+s]=srcdata[x*source->samplesperpixel+s];
			rowbuffer[(x+1)*samplesperpixel-1]=
				(srcdata[(x+1)*source->samplesperpixel-1]*(IS_SAMPLEMAX-maskdata[x]))/IS_SAMPLEMAX;
		}
	}
	else
	{
		for(int x=0;x<width;++x)
		{
			for(int s=0;s<source->samplesperpixel;++s)
				rowbuffer[x*samplesperpixel+s]=srcdata[x*source->samplesperpixel+s];
			rowbuffer[(x+1)*samplesperpixel-1]=IS_SAMPLEMAX-maskdata[x];
		}
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_Mask::ImageSource_Mask(struct ImageSource *source,ImageSource *mask)
	: ImageSource(source), source(source), mask(mask)
{
	if((source->width!=mask->width)||(source->height!=mask->height))
		throw "Source and mask dimensions must match!";

	if(mask->type!=IS_TYPE_GREY)
		throw "Mask should be a greyscale image";

	switch(source->type)
	{
		case IS_TYPE_RGB:
			type=IS_TYPE_RGBA;
			samplesperpixel=4;
			break;
		case IS_TYPE_CMYK:
			type=IS_TYPE_CMYKA;
			samplesperpixel=5;
			break;
		case IS_TYPE_RGBA:
		case IS_TYPE_CMYKA:
			type=source->type;
			break;
		default:
			throw "IS_MASK: source type unsupported";
			break;
	}

	MakeRowBuffer();
}
