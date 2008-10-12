/*
 * imagesource_cms.cpp
 * ImageSource Colour Management filter.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Clean up handling of image types other than RGB
 */

#include <iostream>
#include <stdlib.h>

#include <math.h>

#include "imagesource_cms.h"

using namespace std;

ImageSource_CMS::~ImageSource_CMS()
{
	free(tmp1);
	free(tmp2);

	if(source)
		delete source;

	if(disposetransform)
		delete transform;
}


ISDataType *ImageSource_CMS::GetRow(int row)
{
	ISDataType *src;

	if(row==currentrow)
		return(rowbuffer);

	src=source->GetRow(row);

	// Copy just the colour data from src to tmp1, ignoring alpha
	// (FIXME: separate code-path for the non-alpha case would be quicker)
	for(int i=0;i<width;++i)
	{
		for(int j=0;j<tmpsourcespp;++j)
			tmp1[i*tmpsourcespp+j]=(65535*src[i*source->samplesperpixel+j])/IS_SAMPLEMAX;
	}

	transform->Transform(tmp1,tmp2,width);

	// Copy just the colour data from tmp2 to rowbuffer, ignoring alpha.
	for(int i=0;i<width;++i)
	{
		for(int j=0;j<tmpdestspp;++j)
			rowbuffer[i*samplesperpixel+j]=(IS_SAMPLEMAX*tmp2[i*tmpdestspp+j])/65535;
	}

	// Copy alpha channel unchanged if present
	if(HAS_ALPHA(source->type))
	{
		for(int i=0;i<width;++i)
			rowbuffer[(i+1)*samplesperpixel-1]=src[(i+1)*source->samplesperpixel-1];
	}

	currentrow=row;
	return(rowbuffer);
}

#if 0
ImageSource_CMS::ImageSource_CMS(ImageSource *source,CMSDB &inp,CMSDB &outp)
	: ImageSource(source), source(source)
{
	transform=new CMSTransform(inp,outp);
	disposetransform=true;

	Init();
}

ImageSource_CMS::ImageSource_CMS(ImageSource *source,CMSProfile *inp,CMSDB &outp)
	: ImageSource(source), source(source)
{
	transform=new CMSTransform(inp,outp);
	disposetransform=true;

	Init();
}
#endif


ImageSource_CMS::ImageSource_CMS(ImageSource *source,CMSProfile *inp,CMSProfile *outp)
	: ImageSource(source), source(source)
{
	transform=new CMSTransform(inp,outp);
	disposetransform=true;

	Init();
}


ImageSource_CMS::ImageSource_CMS(ImageSource *source,CMSTransform *transform)
	: ImageSource(source), source(source), transform(transform)
{
	disposetransform=false;

	Init();
}


void ImageSource_CMS::Init()
{
//	cerr << "Initialsing CMS transform" << endl;
	switch(type=transform->GetOutputColourSpace())
	{
		case IS_TYPE_GREY:
			samplesperpixel=1;
			break;
		case IS_TYPE_RGB:
			samplesperpixel=3;
			break;
		case IS_TYPE_CMYK:
			samplesperpixel=4;
			break;
		default:
			throw "Unsupported colour space (output)";
			break;
	}

//	cerr << "Type: " << type << endl;

	if(transform->GetInputColourSpace()!=STRIP_ALPHA(source->type))
	{
		throw "Source image must match source profile!";
	}

	tmpsourcespp=source->samplesperpixel;
	tmpdestspp=samplesperpixel;
	if(HAS_ALPHA(source->type))
	{
		++samplesperpixel;
		--tmpsourcespp;
		type=IS_TYPE(type | IS_TYPE_ALPHA);
	}

	tmp1=(unsigned short *)malloc(sizeof(unsigned short)*width*source->samplesperpixel);
	tmp2=(unsigned short *)malloc(sizeof(unsigned short)*width*samplesperpixel);

//	cerr << "tmpsourcespp: " << tmpsourcespp << endl;
//	cerr << "tmpdestspp: " << tmpdestspp << endl;
//	cerr << "samplesperpixel: " << samplesperpixel << endl;

	MakeRowBuffer();
}
