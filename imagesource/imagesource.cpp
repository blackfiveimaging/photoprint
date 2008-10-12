#include <iostream>

#include "lcmswrapper.h"
#include "imagesource.h"

using namespace std;

ImageSource::ImageSource() : embeddedprofile(NULL), embprofowned(false), rowbuffer(NULL)
{
}


ImageSource::ImageSource(ImageSource *src) : embprofowned(false), rowbuffer(NULL)
{
	width=src->width;
	height=src->height;
	type=src->type;
	samplesperpixel=src->samplesperpixel;
	xres=src->xres;
	yres=src->yres;
	randomaccess=src->randomaccess;
	embeddedprofile=src->embeddedprofile;
	currentrow=-1;
}


ImageSource::~ImageSource()
{
	if(rowbuffer)
		free(rowbuffer);
	if(embeddedprofile && embprofowned)
		delete embeddedprofile;
}


void ImageSource::MakeRowBuffer()
{
	rowbuffer=(ISDataType *)malloc(sizeof(ISDataType)*width*samplesperpixel);
	currentrow=-1;
}


void ImageSource::SetResolution(double xr,double yr)
{
	xres=xr;
	yres=yr;
}


CMSProfile *ImageSource::GetEmbeddedProfile()
{
	return(embeddedprofile);
}


void ImageSource::SetEmbeddedProfile(CMSProfile *profile,bool owned)
{
	if(embeddedprofile && embprofowned)
		delete embeddedprofile;
	embeddedprofile=profile;
	embprofowned=owned;
}

