/*
 * imagesource_solid.cpp
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

#include "imagesource_solid.h"

using namespace std;

ImageSource_Solid::~ImageSource_Solid()
{
}


ISDataType *ImageSource_Solid::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	for(int x=0;x<width;++x)
	{
		for(i=0;i<samplesperpixel;++i)
		{
			rowbuffer[x*samplesperpixel+i]=solid[i];
		}
	}
	currentrow=row;

	return(rowbuffer);
}


ImageSource_Solid::ImageSource_Solid(IS_TYPE type,int width,int height,ISDataType *sld)
	: ImageSource()
{
	this->width=width;
	this->height=height;
	this->type=type;

	switch(type)
	{
		case IS_TYPE_GREY:
			samplesperpixel=1;
			break;
		case IS_TYPE_RGB:
			samplesperpixel=3;
			break;
		case IS_TYPE_CMYK:
			samplesperpixel=4;
			break;
		case IS_TYPE_GREYA:
			samplesperpixel=2;
			break;
		case IS_TYPE_RGBA:
			samplesperpixel=4;
			break;
		case IS_TYPE_CMYKA:
			samplesperpixel=5;
			break;
		default:
			throw "Solid: Type not supported.";
	}
	if(sld)
	{
		for(int i=0;i<samplesperpixel;++i)
		{
			solid[i]=sld[i];
		}
	}
	else
	{
		for(int i=0;i<samplesperpixel;++i)
		{
			solid[i]=0;
		}
	}	
	MakeRowBuffer();
}
