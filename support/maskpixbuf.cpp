#include <iostream>

#include <string.h>

#include "rotatepixbuf.h"
#include "maskpixbuf.h"

using namespace std;

#define OFFSET(pb, x, y) ((x) * (gdk_pixbuf_get_n_channels(pb)) + (y) * gdk_pixbuf_get_rowstride(pb))
 
void maskpixbuf(GdkPixbuf *img,int xpos,int ypos,int width,int height,const GdkPixbuf *mask,
	int redbg8,int greenbg8,int bluebg8)
{
	int bpp=gdk_pixbuf_get_n_channels(img);
	if(bpp==1)
		redbg8=(redbg8+greenbg8+bluebg8)/3;
	if(bpp>4)
		throw "maskpixbuf: More than 4 samples per pixel not supported!";

	int background[4];
	background[0]=redbg8;
	background[1]=greenbg8;
	background[2]=bluebg8;
	background[3]=255;

	GdkPixbuf *scaledmask;
	
	if((width<height)^(gdk_pixbuf_get_width(mask)<gdk_pixbuf_get_height(mask)))
	{
		GdkPixbuf *tmp=gdk_pixbuf_rotate_simple(mask,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
		scaledmask=gdk_pixbuf_scale_simple(tmp,width,height,GDK_INTERP_NEAREST);
		g_object_unref(G_OBJECT(tmp));
	}
	else
		scaledmask=gdk_pixbuf_scale_simple(mask,width,height,GDK_INTERP_NEAREST);
	
	switch(gdk_pixbuf_get_n_channels(scaledmask))
	{
		case 1:
			cerr << "8 bit mask" << endl;
			for(int y=0;y<height;++y)
			{
				guchar *src=gdk_pixbuf_get_pixels(img)+gdk_pixbuf_get_rowstride(img)*(y+ypos);
				guchar *mask=gdk_pixbuf_get_pixels(scaledmask)+gdk_pixbuf_get_rowstride(scaledmask)*y;
				for(int x=0;x<width;++x)
				{
					int sx=x+xpos;
					guchar m=mask[x];
					for(int s=0;s<bpp;++s)
					{
						int p=src[sx*bpp+s];
						p=((p*m)+(background[s]*(255-m)))/255;
						src[sx*bpp+s]=p;
					}
				}
			}
			break;
		case 3:
			for(int y=0;y<height;++y)
			{
				guchar *src=gdk_pixbuf_get_pixels(img)+gdk_pixbuf_get_rowstride(img)*(y+ypos);
				guchar *mask=gdk_pixbuf_get_pixels(scaledmask)+gdk_pixbuf_get_rowstride(scaledmask)*y;
				for(int x=0;x<width;++x)
				{
					int sx=x+xpos;
					guchar m=(mask[x*3]+mask[x*3+1]+mask[x*3+2])/3;
					for(int s=0;s<bpp;++s)
					{
						int p=src[sx*bpp+s];
						p=((p*m)+(background[s]*(255-m)))/255;
						src[sx*bpp+s]=p;
					}
				}
			}
			break;
		case 4:
			for(int y=0;y<height;++y)
			{
				guchar *src=gdk_pixbuf_get_pixels(img)+gdk_pixbuf_get_rowstride(img)*(y+ypos);
				guchar *mask=gdk_pixbuf_get_pixels(scaledmask)+gdk_pixbuf_get_rowstride(scaledmask)*y;
				for(int x=0;x<width;++x)
				{
					int sx=x+xpos;
					guchar m=(mask[x*4]+mask[x*4+1]+mask[x*4+2])/3;
					for(int s=0;s<bpp;++s)
					{
						int p=src[sx*bpp+s];
						p=((p*m)+(background[s]*(255-m)))/255;
						src[sx*bpp+s]=p;
					}
				}
			}
			break;
	}
	g_object_unref(G_OBJECT(scaledmask));
}
