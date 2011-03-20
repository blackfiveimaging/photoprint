/*
 * imagesource_segmentmask.cpp
 * Supports Random Access
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "imagesource_segmentmask.h"
#include "circlemontage.h"

using namespace std;


ImageSource_SegmentMask::~ImageSource_SegmentMask()
{
	if(segment)
		delete segment;
}


ISDataType *ImageSource_SegmentMask::GetRow(int row)
{
	ISDataType *dst;

	if(currentrow==row)
		return(rowbuffer);

	dst=rowbuffer;

	float t1=segment->t1;
	float t2=segment->t2;

	float overlap=segment->overlap;

	int dy=row-segment->yo;
	if(fade)
	{
		for(int x=0;x<width;++x)
		{
			int dx=x-segment->xo;
			int dd=int(sqrt(float(dx*dx+dy*dy)));
	
			if((dd<segment->radius) && (dd>=segment->innerradius))
			{
				float t=(atan2f(dx,-dy)*360)/(2*M_PI);
				if(t1>0.0 && t<0.0)
					t+=360.0;
				if(t>=t1 && t<=t2)
				{
					if(t<(t1+overlap*2))
					{
						*dst++=IS_SAMPLEMAX-ISDataType(IS_SAMPLEMAX*(t1-t)/(overlap*2));
					}
					else if(t>(t2-overlap*2))
					{
						*dst++=IS_SAMPLEMAX-ISDataType(IS_SAMPLEMAX*(t-t2)/(overlap*2));
					}
					else
						*dst++=IS_SAMPLEMAX;
				}
				else
					*dst++=0;
			}
			else
			{
				*dst++=0;
			}
		}
	}
	else
	{
		for(int x=0;x<width;++x)
		{
			int dx=x-segment->xo;
			int dd=int(sqrt(float(dx*dx+dy*dy)));
	
			if((dd<segment->radius) && (dd>=segment->innerradius))
			{
				int t=int((atan2f(dx,-dy)*360)/(2*M_PI));
				if(t1>0 && t<0)
					t+=360;
				if(t>=t1 && t<=t2)
				{
					*dst++=IS_SAMPLEMAX;
				}
				else
					*dst++=0;
			}
			else
			{
				*dst++=0;
			}
		}

	}
	
	currentrow=row;
	return(rowbuffer);
}


ImageSource_SegmentMask::ImageSource_SegmentMask(CMSegment *seg,bool fade)
	: ImageSource(), segment(seg), fade(fade)
{
	type=IS_TYPE_GREY;
	samplesperpixel=1;

	width=seg->w;
	height=seg->h;

	MakeRowBuffer();
	randomaccess=true;
}

