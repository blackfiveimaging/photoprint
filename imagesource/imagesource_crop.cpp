/*
 * imagesource_crop.cpp
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

#include "imagesource_crop.h"

using namespace std;

ImageSource_Crop::~ImageSource_Crop()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Crop::GetRow(int row)
{
	int i;

	// If random access is not supported, we must loop through the unwanted rows.
	if(row==0 && source->randomaccess==false)
	{
		for(int i=0;i<yoffset;++i)
		{
			ISDataType *junk=source->GetRow(i);
		}
	}

	if(row==currentrow)
		return(rowbuffer);

	int srcrow=row+yoffset;
	ISDataType *srcdata=source->GetRow(srcrow);

	switch(source->samplesperpixel)
	{
		case 1:
			for(i=0;i<width;++i)
			{
				int sx=i+xoffset;
				rowbuffer[i]=srcdata[sx];
			}
			break;
		case 3:
			for(i=0;i<width;++i)
			{
				int sx=i+xoffset;
				rowbuffer[i*3]=srcdata[sx*3];
				rowbuffer[i*3+1]=srcdata[sx*3+1];
				rowbuffer[i*3+2]=srcdata[sx*3+2];
			}
			break;
		case 4:
			for(i=0;i<width;++i)
			{
				int sx=i+xoffset;
				rowbuffer[i*4]=srcdata[sx*4];
				rowbuffer[i*4+1]=srcdata[sx*4+1];
				rowbuffer[i*4+2]=srcdata[sx*4+2];
				rowbuffer[i*4+3]=srcdata[sx*4+3];
			}
			break;
	}

	currentrow=row;

	return(rowbuffer);
}


ImageSource_Crop::ImageSource_Crop(struct ImageSource *source,int xoffset,int yoffset,int cropwidth,int cropheight)
	: ImageSource(source), source(source), xoffset(xoffset), yoffset(yoffset)
{
	width=cropwidth;
	height=cropheight;
	if(width>(source->width-xoffset))
		width=source->width-xoffset;
	if(height>(source->height-yoffset))
		height=source->height-yoffset;

	MakeRowBuffer();
}
