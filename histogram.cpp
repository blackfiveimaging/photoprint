#include <iostream>
#include <cmath>

#include <gdk/gdkpixbuf.h>

#include "debug.h"

#include "histogram.h"

using namespace std;

struct Hist_Shades
{
	int r;
	int g;
	int b;
};


// Fixme - no reason why we couldn't create these on the fly from a
// DeviceNColorants.

static Hist_Shades Hist_GreyShades[]=
{
	{255,255,255},	// None
	{128,128,128},	// Black
	{255,255,255},	// As above but with alpha
	{64,64,64},	// Black
};


static Hist_Shades Hist_RGBShades[]=
{
	{255,255,255},	// None
	{255,64,64},	// Red
	{64,255,64},	// Green
	{96,144,0},		// Red + Green
	{64,64,255},	// Blue
	{96,0,144},		// Red + Blue
	{0,96,144},		// Green + Blue
	{64,64,96},		// Red + Green + Blue
	{128,128,128},	// As above but with Alpha
	{128,32,32},	// ...
	{32,128,32},
	{48,72,0},
	{32,32,128},
	{48,0,72},
	{0,48,72},
	{32,32,48},
};


static Hist_Shades Hist_CMYKShades[]=
{
	{255,255,255},	// None
	{64,255,255},	// Cyan
	{255,64,255},	// Magenta
	{96,96,255},	// Cyan + Magenta
	{255,255,64},	// Yellow
	{96,255,96},	// Cyan + Yellow
	{255,96,96},	// Magenta + Yellow
	{128,128,64},		// Cyan + Magenta + Yellow
	{128,128,128},	// As above but with Black
	{32,128,128},	// ...
	{128,32,128},
	{48,48,128},
	{128,128,32},
	{48,128,48},
	{128,48,48},
	{72,72,48}
};


// DrawHistogram() - renders a GdkPixbuf from a histogram.
GdkPixbuf *PPHistogram::DrawHistogram(int width,int height)
{
	Hist_Shades *shades;
	switch(GetType())
	{
		case IS_TYPE_RGB:
		case IS_TYPE_RGBA:
			Debug[TRACE] << "Drawing histogram for RGB Image" << endl;
			shades=Hist_RGBShades;
			break;
		case IS_TYPE_CMYK:
			Debug[TRACE] << "Drawing histogram for CMYK Image" << endl;
			shades=Hist_CMYKShades;
			break;
		case IS_TYPE_GREY:
			Debug[TRACE] << "Drawing histogram for GREY Image" << endl;
			shades=Hist_GreyShades;
			break;
		default:
			throw "Unknown histogram type";
			break;
	}
	int channels=GetChannelCount();
	GdkPixbuf *pb=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,width,height);

	if(pb)
	{
		int rowstride=gdk_pixbuf_get_rowstride(pb);
		unsigned char *pixels=gdk_pixbuf_get_pixels(pb);
		double max=GetMax();

		for(int x=0;x<width;++x)
		{
			int bucket=(x*IS_HISTOGRAM_BUCKETS)/width;
			for(int y=0;y<height;++y)
			{
				int bit=1;
				int ci=0;
				for(int c=0;c<channels;++c)
				{
					double t=(*this)[c][bucket];
					t/=max;
					t=sqrt(t);
					if((height-y)<(255.0*t))
						ci|=bit;
					bit<<=1;
				}
				int pi=3*x+y*rowstride;
				pixels[pi]=shades[ci].r;
				pixels[pi+1]=shades[ci].g;
				pixels[pi+2]=shades[ci].b;
			}
		}
	}
	return(pb);
}

