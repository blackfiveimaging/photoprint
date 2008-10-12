/*
 * imagesource_flatten.cpp
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

#include "imagesource_flatten.h"

using namespace std;

ImageSource_Flatten::~ImageSource_Flatten()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Flatten::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	if(!HAS_ALPHA(source->type))
		return(srcdata);

	switch(samplesperpixel)
	{
		case 1:
			for(i=0;i<width;++i)
			{
				int a=srcdata[i*2+1];
				rowbuffer[i]=(a*srcdata[i*2]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
			}
			break;
		case 3:
			for(i=0;i<width;++i)
			{
				int a=srcdata[i*4+3];
				rowbuffer[i*3]=(a*srcdata[i*4]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
				rowbuffer[i*3+1]=(a*srcdata[i*4+1]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
				rowbuffer[i*3+2]=(a*srcdata[i*4+2]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
			}
			break;
		case 4:
			for(i=0;i<width;++i)
			{
				int a=srcdata[i*5+4];
				rowbuffer[i*4]=(a*srcdata[i*5]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
				rowbuffer[i*4+1]=(a*srcdata[i*5+1]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
				rowbuffer[i*4+2]=(a*srcdata[i*5+2]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
				rowbuffer[i*4+3]=(a*srcdata[i*5+3]+IS_SAMPLEMAX*(IS_SAMPLEMAX-a))/IS_SAMPLEMAX;
			}
			break;
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_Flatten::ImageSource_Flatten(struct ImageSource *source)
	: ImageSource(source), source(source)
{
	if(HAS_ALPHA(type))
	{
		type=IS_TYPE(STRIP_ALPHA(type));
		--samplesperpixel;
	}
	MakeRowBuffer();
}
