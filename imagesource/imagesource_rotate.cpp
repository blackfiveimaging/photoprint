/*
 * imagesource_rotate.cpp - filter to rotate and image through
 * 0, 90, 180 or 270 degrees.
 * supports random access, unless source image doesn't, and rotation is zero.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <stdlib.h>

#include "imagesource_rotate.h"

using namespace std;

ImageSource_Rotate::~ImageSource_Rotate()
{
	if(source)
		delete source;
	if(spanbuffer)
		free(spanbuffer);
}


ISDataType *ImageSource_Rotate::GetRow(int row)
{
	int x;
	int firstrow,lastrow;
	ISDataType *dst;
	ISDataType *src;
	ISDataType c;
	
	switch(rotation)
	{
		case 0:
			// FIXME - if source doesn't support random access, image needs
			// to be cached.
			return(source->GetRow(row));
			break;
		case 90:
			if((row<spanfirstrow) || (row>=(spanfirstrow+spanrows)))
			{
				spanfirstrow=row;

				firstrow=row;
				lastrow=row+spanrows;
				if(lastrow>height)
					lastrow=height;

				for(int i=0;i<source->height;++i)
				{
					src=source->GetRow(i);
					dst=spanbuffer+samplesperpixel*i;
					switch(samplesperpixel)
					{
						case 1:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=(source->width-1)-x;
								c=src[sx];
								dst[(x-firstrow)*samplesperrow]=c;
							}
							break;
						case 3:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=(source->width-1)-x;
								c=src[sx*3];
								dst[(x-firstrow)*samplesperrow]=c;
								c=src[sx*3+1];
								dst[(x-firstrow)*samplesperrow+1]=c;
								c=src[sx*3+2];
								dst[(x-firstrow)*samplesperrow+2]=c;
							}
							break;
						case 4:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=(source->width-1)-x;
								c=src[sx*4];
								dst[(x-firstrow)*samplesperrow]=c;
								c=src[sx*4+1];
								dst[(x-firstrow)*samplesperrow+1]=c;
								c=src[sx*4+2];
								dst[(x-firstrow)*samplesperrow+2]=c;
								c=src[sx*4+3];
								dst[(x-firstrow)*samplesperrow+3]=c;
							}
							break;
						default:
							throw "Rotate: Only 1, 3 or 4 samples per pixel are currently supported";
							break;
					}
				}
			}
			row-=spanfirstrow;
			return(spanbuffer+row*samplesperrow);		
			break;
		case 180:
			// FIXME: support partial image caching here.

			if((row<spanfirstrow) || (row>=(spanfirstrow+spanrows)))
			{
				spanfirstrow=row;

				firstrow=row;
				lastrow=row+spanrows;
				if(lastrow>height)
					lastrow=height;

				for(int y=0;y<height;++y)
				{
					src=source->GetRow(y);
					dst=spanbuffer+((height-1)-y)*samplesperrow;
					switch(samplesperpixel)
					{
						case 1:
							for(x=0;x<width;++x)
							{
								int sx=(width-1)-x;
								c=src[sx];
								dst[x]=c;
							}
							break;
						case 3:
							for(x=0;x<width;++x)
							{
								int sx=(width-1)-x;
								c=src[sx*3];
								dst[x*3]=c;
								c=src[sx*3+1];
								dst[x*3+1]=c;
								c=src[sx*3+2];
								dst[x*3+2]=c;
							}
							break;
						case 4:
							for(x=0;x<width;++x)
							{
								int sx=(width-1)-x;
								c=src[sx*4];
								dst[x*4]=c;
								c=src[sx*4+1];
								dst[x*4+1]=c;
								c=src[sx*4+2];
								dst[x*4+2]=c;
								c=src[sx*4+3];
								dst[x*4+3]=c;
							}
							break;
						default:
							throw "Rotate: Only 1, 3 or 4 samples per pixel are currently supported";
							break;
					}
				}
			}
			row-=spanfirstrow;
			return(spanbuffer+row*samplesperrow);		
			break;
			
#if 0
			src=source->GetRow((height-1)-row);
			dst=rowbuffer;

			switch(samplesperpixel)
			{
				case 1:
					for(x=0;x<width;++x)
					{
						int sx=(width-1)-x;
						c=src[sx];
						dst[x]=c;
					}
					break;
				case 3:
					for(x=0;x<width;++x)
					{
						int sx=(width-1)-x;
						c=src[sx*3];
						dst[x*3]=c;
						c=src[sx*3+1];
						dst[x*3+1]=c;
						c=src[sx*3+2];
						dst[x*3+2]=c;
					}
					break;
				case 4:
					for(x=0;x<width;++x)
					{
						int sx=(width-1)-x;
						c=src[sx*4];
						dst[x*4]=c;
						c=src[sx*4+1];
						dst[x*4+1]=c;
						c=src[sx*4+2];
						dst[x*4+2]=c;
						c=src[sx*4+3];
						dst[x*4+3]=c;
					}
					break;
				default:
					throw "Rotate: Only 1, 3 or 4 samples per pixel are currently supported";
					break;
			}
#endif
			break;
		case 270:
			if((row<spanfirstrow) || (row>=(spanfirstrow+spanrows)))
			{
				spanfirstrow=row;
		
				firstrow=row;
				lastrow=row+spanrows;
				if(lastrow>height)
					lastrow=height;
		
				for(int i=source->height-1;i>=0;--i)
				{
					src=source->GetRow((source->height-1)-i);
					dst=spanbuffer+samplesperpixel*i;
					switch(samplesperpixel)
					{
						case 1:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=x;
								c=src[sx];
								dst[(x-firstrow)*samplesperrow]=c;
							}
							break;
						case 3:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=x;
								c=src[sx*3];
								dst[(x-firstrow)*samplesperrow]=c;
								c=src[sx*3+1];
								dst[(x-firstrow)*samplesperrow+1]=c;
								c=src[sx*3+2];
								dst[(x-firstrow)*samplesperrow+2]=c;
							}
							break;
						case 4:
							for(x=firstrow;x<lastrow;++x)
							{
								int sx=x;
								c=src[sx*4];
								dst[(x-firstrow)*samplesperrow]=c;
								c=src[sx*4+1];
								dst[(x-firstrow)*samplesperrow+1]=c;
								c=src[sx*4+2];
								dst[(x-firstrow)*samplesperrow+2]=c;
								c=src[sx*4+3];
								dst[(x-firstrow)*samplesperrow+3]=c;
							}
							break;
						default:
							throw "Rotate: Only 1, 3 or 4 samples per pixel are currently supported";
							break;
					}
				}
			}
			row-=spanfirstrow;
			return(spanbuffer+row*samplesperrow);
			break;
		default:
			throw "Currently only multples of 90 degrees are supported";
	}
	return(rowbuffer);
}


ImageSource_Rotate::ImageSource_Rotate(ImageSource *source,int rotation,int spanrows)
	: ImageSource(source), source(source), rotation(rotation), spanfirstrow(0), spanrows(spanrows), spanbuffer(NULL)
{
	rowbuffer=NULL;
	switch(rotation)
	{
		case 0:
		case 180:
			break;
		case 90:
		case 270:
			width=source->height;
			height=source->width;
			xres=source->yres;
			yres=source->xres;
			break;
		default:
			throw "Only multiples of 90 degrees are supported\n";
			break;
	}

	switch(rotation)
	{
		case 0:
		case 180:
			cerr << "Rotate: caching entire image for 180 degree rotation" << endl;
			this->spanrows=source->height+1;
			break;
		case 90:
		case 270:
			if(!source->randomaccess)
			{
				cerr << "Rotate: source doesn't support random access - caching entire image" << endl;
				this->spanrows=source->width+1;
			}
			break;
	}

	spanfirstrow=-this->spanrows-1;
	samplesperrow=width*samplesperpixel;

	cerr << "Span buffer will be " << this->spanrows << " high" << endl;

	if((rotation==90) || (rotation==270) || (rotation==180))
		spanbuffer=(ISDataType *)malloc(this->spanrows*(sizeof(ISDataType)*samplesperrow));
	else
		MakeRowBuffer();
	currentrow=-1;
	if(rotation==0)
		randomaccess=source->randomaccess;
	else
		randomaccess=true;
}
