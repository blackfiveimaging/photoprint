/* Imagesource_Downsample - a sub-pixel box-filter downsampling routine
   (c) 2004-2008 by Alastair M. Robinson
   Re-implemented from scratch using a better algorithm 2008-05-17 */

#include <iostream>
#include <stdlib.h>
#include <string.h>

#include "imagesource_downsample.h"

using namespace std;


// Downsampling is implemented as a chaining of a horizontal, then a vertical downsampling.


ImageSource_Downsample::~ImageSource_Downsample()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_Downsample::GetRow(int row)
{
	return(source->GetRow(row));
}


ImageSource_Downsample::ImageSource_Downsample(struct ImageSource *source,int width,int height)
	: ImageSource(source), source(source)
{
	this->source=new ImageSource_HDownsample(this->source,width);
	this->source=new ImageSource_VDownsample(this->source,height);
	xres=this->source->xres;
	yres=this->source->yres;
	this->randomaccess=this->source->randomaccess;
	this->width=width;
	this->height=height;
}


// Horizontal downsampling


ImageSource_HDownsample::~ImageSource_HDownsample()
{
	if(source)
		delete source;
}


ISDataType *ImageSource_HDownsample::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);
	currentrow=row;

	// We accumulate pixel values from a potentially
	// large number of pixels and process all the samples
	// in a pixel at one time.
	double tmp[IS_MAX_SAMPLESPERPIXEL];
	for(int i=0;i<samplesperpixel;++i)
		tmp[i]=0;

	ISDataType *srcdata=source->GetRow(row);

	// We use a Bresenham-esque method of calculating the
	// pixel boundaries for scaling - add the smaller value
	// to an accumulator until it exceeds the larger value,
	// then subtract the larger value, leaving the remainder
	// in place for the next round.
	int a=0;
	int src=0;
	int dst=0;
	while(dst<width)
	{
		// Add the smaller value (destination width)
		a+=width;

		// As long as the counter is less than the larger value
		// (source width), we take full pixels.
		while(a<source->width)
		{
			if(src>=source->width)
				src=source->width-1;
			for(int i=0;i<samplesperpixel;++i)
				tmp[i]+=srcdata[samplesperpixel*src+i];
			++src;
			a+=width;
		}

		double p=source->width-(a-width);
		p/=width;
		// p now contains the proportion of the next pixel
		// to be counted towards the output pixel.

		a-=source->width;
		// And a now contains the remainder,
		// ready for the next round.

		// So we add p * the new source pixel
		// to the current output pixel...
		if(src>=source->width)
			src=source->width-1;
		for(int i=0;i<samplesperpixel;++i)
			tmp[i]+=p*srcdata[samplesperpixel*src+i];

		// Store it...
		for(int i=0;i<samplesperpixel;++i)
		{
			rowbuffer[samplesperpixel*dst+i] =
				0.5+(tmp[i]*width)/source->width;
		}
		++dst;

		// And start off the next output pixel with
		// (1-p) * the source pixel.
		for(int i=0;i<samplesperpixel;++i)
			tmp[i]=(1.0-p)*srcdata[samplesperpixel*src+i];
		++src;
	}

	return(rowbuffer);
}


ImageSource_HDownsample::ImageSource_HDownsample(struct ImageSource *source,int width)
	: ImageSource(source), source(source)
{
	cerr << "Using hdownsample filter" << endl;
	this->width=width;
	xres=(source->xres*width); xres/=source->width;

	MakeRowBuffer();
}


// Vertical downsampling routine


ImageSource_VDownsample::~ImageSource_VDownsample()
{
	if(source)
		delete source;

	if(tmp)
		free(tmp);
}


ISDataType *ImageSource_VDownsample::GetRow(int row)
{
	if(currentrow==row)
		return(rowbuffer);
	currentrow=row;

	ISDataType *srcdata;

	// Add the smaller value (destination width)
	acc+=height;

	// As long as the counter is less than the larger value, we take full pixels.
	while(acc<source->height)
	{
		if(srcrow>=source->height)
			srcrow=source->height-1;
		srcdata=source->GetRow(srcrow++);
		for(int i=0;i<width*samplesperpixel;++i)
			tmp[i]+=srcdata[i];
		acc+=height;
	}

	double p=source->height-(acc-height);
	p/=height;
	// p now contains the proportion of the next row to be counted towards the output row.

	acc-=source->height;
	// And acc now contains the remainder, ready for the next round.

	// So we add p * the new source pixel to the current output pixel...
	if(srcrow>=source->height)
		srcrow=source->height-1;
	srcdata=source->GetRow(srcrow);
	for(int i=0;i<width*samplesperpixel;++i)
		tmp[i]+=p*srcdata[i];

	// Store it...
	for(int i=0;i<width*samplesperpixel;++i)
		rowbuffer[i]=0.5+(tmp[i]*height)/source->height;

	// And start off the next output pixel with (1-p) * the source pixel.
	for(int i=0;i<width*samplesperpixel;++i)
		tmp[i]=(1.0-p)*srcdata[i];
	++srcrow;

	return(rowbuffer);
}


ImageSource_VDownsample::ImageSource_VDownsample(struct ImageSource *source,int height)
	: ImageSource(source), source(source), tmp(NULL), srcrow(0), acc(0)
{
	cerr << "Using vdownsample filter" << endl;
	this->height=height;
	yres=(source->yres*height); yres/=source->height;
	randomaccess=false;
	MakeRowBuffer();
	if(!(tmp=(double *)malloc(sizeof(double)*width*samplesperpixel)))
		throw "Can't allocate scaling buffer!";
	for(int i=0;i<width*samplesperpixel;++i)
		tmp[i]=0.0;
}

