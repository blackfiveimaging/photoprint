/*
 * pixbuf_from_imagesource.cpp
 * Creates a GdkPixbuf from an ImageSource
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include "pixbuf_from_imagesource.h"

using namespace std;

GdkPixbuf *pixbuf_from_imagesource(ImageSource *is,
	int redbg8,int greenbg8,int bluebg8,Progress *prog)
{
	if(!is)
		return(NULL);

	GdkPixbuf *pb;

	switch(IS_TYPE_RGB)
	{
		case IS_TYPE_RGB:
		case IS_TYPE_RGBA:
		case IS_TYPE_GREY:
		case IS_TYPE_GREYA:
		case IS_TYPE_CMYK:
			pb=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,is->width,is->height);
			break;
		default:
			return(NULL);
			break;
	}

	if(pb)
	{
		int rowstride=gdk_pixbuf_get_rowstride(pb);
		unsigned char *pixels=gdk_pixbuf_get_pixels(pb);

		// Displaying the progress meter can be expensive,
		// so we only update it often enough to reflect single
		// percentage steps.
		int progressmodulo=is->height/100;
		if(progressmodulo==0) progressmodulo=1;

		for(int y=0;y<is->height;++y)
		{
			ISDataType *src=is->GetRow(y);
			switch(is->type)
			{
				case IS_TYPE_RGBA:
					for(int x=0;x<is->width;++x)
					{
						int a=255-ISTOEIGHT(src[x*4+3]);
						pixels[x*3]=(ISTOEIGHT(src[x*4])*a+redbg8*(255-a))/255;
						pixels[x*3+1]=(ISTOEIGHT(src[x*4+1])*a+greenbg8*(255-a))/255;
						pixels[x*3+2]=(ISTOEIGHT(src[x*4+2])*a+bluebg8*(255-a))/255;
					}		
					break;
				case IS_TYPE_GREYA:
					for(int x=0;x<is->width;++x)
					{
						int a=255-ISTOEIGHT(src[x*2+3]);
						pixels[x*3]=(ISTOEIGHT(IS_SAMPLEMAX-src[x*2])*a+redbg8*(255-a))/255;
						pixels[x*3+1]=(ISTOEIGHT(IS_SAMPLEMAX-src[x*2])*a+greenbg8*(255-a))/255;
						pixels[x*3+2]=(ISTOEIGHT(IS_SAMPLEMAX-src[x*2])*a+bluebg8*(255-a))/255;
					}		
					break;
				case IS_TYPE_CMYK:
					for(int x=0;x<is->width;++x)
					{
						int pc=ISTOEIGHT(src[x*4]);
						int pm=ISTOEIGHT(src[x*4+1]);
						int py=ISTOEIGHT(src[x*4+2]);
						int pk=ISTOEIGHT(src[x*4+3]);
						int r=(255-pc)-(pk);
						int g=(255-pm)-(pk);
						int b=(255-py)-(pk);
						if(r<0) r=0;
						if(g<0) g=0;
						if(b<0) b=0;
						pixels[x*3]=r;
						pixels[x*3+1]=g;
						pixels[x*3+2]=b;
					}		
					break;
				case IS_TYPE_RGB:
					for(int x=0;x<is->width*is->samplesperpixel;++x)
					{
						pixels[x]=ISTOEIGHT(src[x]);
					}		
					break;
				case IS_TYPE_GREY:
					for(int x=0;x<is->width*is->samplesperpixel;++x)
					{
						pixels[x*3]=ISTOEIGHT(IS_SAMPLEMAX-src[x]);
						pixels[x*3+1]=ISTOEIGHT(IS_SAMPLEMAX-src[x]);
						pixels[x*3+2]=ISTOEIGHT(IS_SAMPLEMAX-src[x]);
					}		
					break;
				default:
					cerr << "pixbuf_from_imagesource: Huh?  IS type of " << is->type << " should have been rejected already." << endl;
					g_object_unref(G_OBJECT(pb));
					return(NULL);
					break;
			}
			pixels+=rowstride;
			bool cont=true;
			if((y%progressmodulo)==0)
			{
				if(prog)
					cont=prog->DoProgress(y,is->height);
			}
			if(!cont)
			{
				g_object_unref(G_OBJECT(pb));
				return(NULL);
			}
		}
	}
	return(pb);
}
