/*
 * imagesource_gaussianblur.h - Blur filter
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GAUSSIANBLUR_H
#define IMAGESOURCE_GAUSSIANBLUR_H

#include "imagesource.h"
#include "convkernel.h"

class ISGaussianBlur_RowCache;

class ImageSource_GaussianBlur : public ImageSource
{
	public:
	ImageSource_GaussianBlur(ImageSource *source,float radius);
	~ImageSource_GaussianBlur();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ConvKernel *kernel;
	int hextra,vextra;
	ISGaussianBlur_RowCache *cache;
	float **tmprows;
	float amount;
	float threshold;
	friend class ISGaussianBlur_RowCache;
};

#endif
