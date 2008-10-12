/*
 * imagesource_desaturate.cpp
 *
 * Supports Greyscale and RGB data
 * Supports random access
 *
 * Copyright (c) 2007 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Support CMYK Data
 *
 */

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "imagesource_desaturate.h"

using namespace std;

ImageSource_Desaturate::~ImageSource_Desaturate()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Desaturate::GetRow(int row)
{
	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	switch(type)
	{
		case IS_TYPE_GREY:
		case IS_TYPE_GREYA:
			// Grey images are already desaturated by definition
			// so no-op.
			return(srcdata);
			break;
		case IS_TYPE_RGB:
			{
				for(int s=0;s<width*samplesperpixel;s+=samplesperpixel)
				{
					int t=(srcdata[s]+srcdata[s+1]+srcdata[s+2])/3;
					rowbuffer[s]=rowbuffer[s+1]=rowbuffer[s+2]=t;
				}
			}
			break;
		case IS_TYPE_RGBA:
			{
				for(int s=0;s<width*samplesperpixel;s+=samplesperpixel)
				{
					int t=(srcdata[s]+srcdata[s+1]+srcdata[s+2])/3;
					rowbuffer[s]=rowbuffer[s+1]=rowbuffer[s+2]=t;
					rowbuffer[s+3]=srcdata[s+3];
				}
			}
			break;
		default:
			throw "Desaturate: type not (yet) handled";
	}
	
	currentrow=row;

	return(rowbuffer);
}


ImageSource_Desaturate::ImageSource_Desaturate(ImageSource *source)
	: ImageSource(source), source(source)
{
	if(STRIP_ALPHA(source->type)==IS_TYPE_CMYK)
		throw "Desaturate: CMYK Images not yet supported.";

	MakeRowBuffer();
}
