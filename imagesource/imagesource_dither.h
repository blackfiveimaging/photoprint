/*
 * imagesource_dither.h
 *
 * Supports Greyscale, RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_DITHER_H
#define IMAGESOURCE_DITHER_H

#include "imagesource.h"

class ImageSource_Dither : public ImageSource
{
	public:
	ImageSource_Dither(ImageSource *source,int bitdepth);
	~ImageSource_Dither();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int *err1;
	int *err2;
	int mask;
};

#endif
