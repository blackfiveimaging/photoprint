/*
 * imagesource_unsharpmask.h - Sharpen filter
 *
 * Copyright (c) 2008 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_UNSHARPMASK_H
#define IMAGESOURCE_UNSHARPMASK_H

#include "imagesource.h"
#include "convkernel.h"

class ISUnsharpMask_RowCache;

class ImageSource_UnsharpMask : public ImageSource
{
	public:
	ImageSource_UnsharpMask(ImageSource *source,float radius,float amount=1.0,float threshold=0.0);
	~ImageSource_UnsharpMask();
	ISDataType *GetRow(int row);
	protected:
	ImageSource *source;
	ConvKernel *kernel;
	int hextra,vextra;
	ISUnsharpMask_RowCache *cache;
	float **tmprows;
	float amount;
	float threshold;
	friend class ISUnsharpMask_RowCache;
};

#endif
