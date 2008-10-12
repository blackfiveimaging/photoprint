/*
 * imagesource_crop.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_CROP_H
#define IMAGESOURCE_CROP_H

#include "imagesource.h"

class ImageSource_Crop : public ImageSource
{
	public:
	ImageSource_Crop(ImageSource *source,int xoffset,int yoffset,int cropwidth,int cropheight);
	~ImageSource_Crop();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int xoffset,yoffset;
};

#endif
