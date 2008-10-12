/*
 * imagesource_flatten.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_FLATTEN_H
#define IMAGESOURCE_FLATTEN_H

#include "imagesource.h"

class ImageSource_Flatten : public ImageSource
{
	public:
	ImageSource_Flatten(ImageSource *source);
	~ImageSource_Flatten();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
