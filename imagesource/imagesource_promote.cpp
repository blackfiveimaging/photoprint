/*
 * imagesource_promote.cpp
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

#include "imagesource_promote.h"

using namespace std;

ImageSource_Promote::~ImageSource_Promote()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Promote::GetRow(int row)
{
	int i;

	if(row==currentrow)
		return(rowbuffer);

	ISDataType *srcdata=source->GetRow(row);

	switch(source->type)
	{
		case IS_TYPE_GREY:
		case IS_TYPE_BW:
			switch(samplesperpixel)
			{
				case 3:
					for(i=0;i<width;++i)
					{
						int g=IS_SAMPLEMAX-srcdata[i];
						rowbuffer[i*3]=g;
						rowbuffer[i*3+1]=g;
						rowbuffer[i*3+2]=g;
					}
					break;
				case 4:
					for(i=0;i<width;++i)
					{
						int g=srcdata[i];
						rowbuffer[i*4]=0;
						rowbuffer[i*4+1]=0;
						rowbuffer[i*4+2]=0;
						rowbuffer[i*4+3]=g;
					}
					break;
			}
			break;
		default:
			throw "Promote: Can't handle source colourspace!"; 
			break;
	}
	currentrow=row;

	return(rowbuffer);
}


ImageSource_Promote::ImageSource_Promote(struct ImageSource *source,IS_TYPE type)
	: ImageSource(source), source(source)
{
	if(HAS_ALPHA(type))
		throw "Promote: Alpha channel not yet supported.";
	this->type=type;
	switch(type)
	{
		case IS_TYPE_RGB:
			samplesperpixel=3;
			break;
		case IS_TYPE_CMYK:
			samplesperpixel=4;
			break;
		default:
			throw "Promote: Alpha channel not yet supported.";
	}
	MakeRowBuffer();
}
