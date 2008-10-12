/*
 * imagesource_mask.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_MASK_H
#define IMAGESOURCE_MASK_H

#include "imagesource.h"

class ImageSource_Mask : public ImageSource
{
	public:
	ImageSource_Mask(ImageSource *source,ImageSource *mask);
	~ImageSource_Mask();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	ImageSource *mask;
};

#endif
