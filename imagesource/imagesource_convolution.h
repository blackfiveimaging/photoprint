/*
 * imagesource_unsharpmask.h - Sharpen filter
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_CONVOLUTION_H
#define IMAGESOURCE_CONVOLUTION_H

#include "imagesource.h"
#include "convkernel.h"

class ISConvolution_RowCache;

class ImageSource_Convolution : public ImageSource
{
	public:
	ImageSource_Convolution(ImageSource *source,ConvKernel *kernel);
	~ImageSource_Convolution();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ConvKernel *kernel;
	int hextra,vextra;
	ISConvolution_RowCache *cache;
	float **tmprows;
	friend class ISConvolution_RowCache;
};

#endif
