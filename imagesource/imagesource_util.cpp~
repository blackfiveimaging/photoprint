#include <iostream>
#include <string.h>

#include "imagesource.h"
#include "imagesource_jpeg.h"
#include "imagesource_tiff.h"
#include "imagesource_bmp.h"
#include "imagesource_pnm.h"
#include "imagesource_gdkpixbuf.h"
#include "imagesource_gs.h"

#include "imagesource_scale.h"
#include "imagesource_downsample.h"
#include "imagesource_bilinear.h"
#include "imagesource_lanczossinc.h"

#include "imagesource_util.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

static const IS_ScalingQualityDescription Descriptions[]=
{
	{N_("Automatic"),N_("Selects a scaling algorithm based on scale factor")},
	{N_("Fast"),N_("Very fast scaling with no interpolation")},
	{N_("Bilinear"),N_("An interpolation filter with mediocre speed and quality")},
	{N_("LanczosSinc"),N_("Slow but very high quality interpolation filter")},
	{NULL,NULL},
	{N_("Downsample"),N_("High quality filter for reductions only")}
};


static const char *findextension(const char *filename)
{
	int t=strlen(filename)-1;
	int c;
	for(c=t;c>0;--c)
	{
		if(filename[c]=='.')
			return(filename+c);
	}
	return(filename);
}

 
ImageSource *ISLoadImage(const char *filename)
{
	const char *ext=findextension(filename);
	try
	{
		cerr << "Loading filename: " << filename << endl;
		cerr << "Extension: " << ext << endl; 
		if(strncasecmp(ext,".JPG",4)==0)
			return(new ImageSource_JPEG(filename));
		else if(strncasecmp(ext,".JPEG",5)==0)
			return(new ImageSource_JPEG(filename));
		else if(strncasecmp(ext,".JFIF",5)==0)
			return(new ImageSource_JPEG(filename));
		else if(strncasecmp(ext,".BMP",4)==0)
			return(new ImageSource_BMP(filename));
		else if(strncasecmp(ext,".TIF",4)==0)
			return(new ImageSource_TIFF(filename));
		else if(strncasecmp(ext,".TIFF",5)==0)
			return(new ImageSource_TIFF(filename));
#if defined HAVE_LIBPNM || defined HAVE_LIBNETPBM
		else if(strncasecmp(ext,".PPM",4)==0)
			return(new ImageSource_PNM(filename));
		else if(strncasecmp(ext,".PGM",4)==0)
			return(new ImageSource_PNM(filename));
		else if(strncasecmp(ext,".PBM",4)==0)
			return(new ImageSource_PNM(filename));
		else if(strncasecmp(ext,".PNM",4)==0)
			return(new ImageSource_PNM(filename));
#endif
		else if(strncasecmp(ext,".PS",4)==0)
			return(new ImageSource_GS(filename,IMAGESOURCE_GS_DEFAULT_RESOLUTION));
		else if(strncasecmp(ext,".PDF",4)==0)
			return(new ImageSource_GS(filename,IMAGESOURCE_GS_DEFAULT_RESOLUTION));
	}
	catch(const char *err)
	{
		cerr << "Attempt to load " << filename << " failed" << endl;
		cerr << "(" << err << ")" << endl;
		cerr << "- falling back to GdkPixbuf loader" << endl;
	}
	return(new ImageSource_GdkPixbuf(filename));
}


ImageSource *ISScaleImageByResolution(ImageSource *source,double xres,double yres,IS_ScalingQuality quality)
{
	ImageSource *result=NULL;
	int w=int((source->width*xres)/source->xres);
	int h=int((source->height*yres)/source->yres);
	
	result=ISScaleImageBySize(source,w,h,quality);

	if(result)
		result->SetResolution(xres,yres);

	return(result);
}


static IS_ScalingQuality ChooseScaling(int src,int dst,IS_ScalingQuality quality)
{
	if(quality==IS_SCALING_AUTOMATIC)
	{
		double f=dst;  f/=src;
		// Use LanczosSinc for scaling >250%,
		// Bilinear for 250% - 100%, and
		// nearest neighbour for scaling <=100%
		if(f>2.5)
			quality=IS_SCALING_LANCZOSSINC;
		else if(f>1.0)
			quality=IS_SCALING_BILINEAR;
		else
			quality=IS_SCALING_DOWNSAMPLE;
	}
	else if ((quality!=IS_SCALING_NEARESTNEIGHBOUR) && (dst<src))
	{
		// If we're downsampling, and the quality hasn't been explicitly set to 
		// nearest neighbour, we use the downsample method;
		quality=IS_SCALING_DOWNSAMPLE;
	}

	return(quality);
}


ImageSource *ISScaleImageBySize(ImageSource *source,int width,int height,IS_ScalingQuality quality)
{
	IS_ScalingQuality hscale=ChooseScaling(source->width,width,quality);
	IS_ScalingQuality vscale=ChooseScaling(source->height,height,quality);

	switch(hscale)
	{
		default:
		case IS_SCALING_NEARESTNEIGHBOUR:
			source=new ImageSource_HScale(source,width);
			break;
		case IS_SCALING_BILINEAR:
			source=new ImageSource_HBilinear(source,width);
			break;
		case IS_SCALING_LANCZOSSINC:
			// We could perform a slight downsampling before the LanczosSinc scaling
			// to reduce ringing artifacts, at the expense of a small amount
			// of sharpness
//			source=new ImageSource_HDownsample(source,(source->width*95)/100);
			source=new ImageSource_HLanczosSinc(source,width);
			break;
		case IS_SCALING_DOWNSAMPLE:
			source=new ImageSource_HDownsample(source,width);
			break;
	}

	switch(vscale)
	{
		default:
		case IS_SCALING_NEARESTNEIGHBOUR:
			source=new ImageSource_VScale(source,height);
			break;
		case IS_SCALING_BILINEAR:
			source=new ImageSource_VBilinear(source,height);
			break;
		case IS_SCALING_LANCZOSSINC:
			// We could perform a slight downsampling before the LanczosSinc scaling
			// to reduce ringing artifacts, at the expense of a small amount
			// of sharpness
//			source=new ImageSource_VDownsample(source,(source->height*95)/100);
			source=new ImageSource_VLanczosSinc(source,height);
			break;
		case IS_SCALING_DOWNSAMPLE:
			source=new ImageSource_VDownsample(source,height);
			break;
	}

#if 0
	// don't use an expensive scaling function if the image is being reduced...
	if((width<source->width) && (height<source->height))
	{
		switch(quality)
		{
			case IS_SCALING_NEARESTNEIGHBOUR:
				cerr << "Image is being shrunk - using Nearest Neighbour scaling" << endl;
				quality=IS_SCALING_NEARESTNEIGHBOUR;
				break;
			default:
				cerr << "Using Downsample filter..." << endl;
				quality=IS_SCALING_DOWNSAMPLE;
				break;
		}
	}

	switch(quality)
	{
		case IS_SCALING_NEARESTNEIGHBOUR:
		case IS_SCALING_LANCZOSSINC:
		case IS_SCALING_BILINEAR:
		case IS_SCALING_DOWNSAMPLE:
			break;
		case IS_SCALING_AUTOMATIC:
		default:
			{
				double xf=width; xf/=source->width;
				double yf=height; yf/=source->height;
				double f=xf;
				if(yf>f) f=yf;
				// Use LanczosSinc for scaling >250%,
				// Bilinear for 250% - 100%, and
				// nearest neighbour for scaling <=100%
				if(f>2.5)
					quality=IS_SCALING_LANCZOSSINC;
				else if(f>1.0)
					quality=IS_SCALING_BILINEAR;
				else
					quality=IS_SCALING_NEARESTNEIGHBOUR;
			}
	}

	switch(quality)
	{
		default:
		case IS_SCALING_NEARESTNEIGHBOUR:
			result=new ImageSource_Scale(source,width,height);
			break;
		case IS_SCALING_BILINEAR:
			result=new ImageSource_Bilinear(source,width,height);
			break;
		case IS_SCALING_LANCZOSSINC:
			result=new ImageSource_LanczosSinc(source,width,height);
			break;
		case IS_SCALING_DOWNSAMPLE:
			result=new ImageSource_Downsample(source,width,height);
			break;
	}
#endif
	return(source);
}


const IS_ScalingQualityDescription *DescribeScalingQuality(IS_ScalingQuality quality)
{
	if(quality<IS_SCALING_MAX)
		return(&Descriptions[quality]);
	else
		return(NULL);
}
