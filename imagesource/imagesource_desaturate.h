/*
 * imagesource_desaturate.h
 *
 * Supports Grey and RGB data
 * Supports random access
 *
 * Copyright (c) 2007 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Support CMYK Data
 *
 */

#ifndef IMAGESOURCE_DESATURATE_H
#define IMAGESOURCE_DESATURATE_H

#include "imagesource.h"

class ImageSource_Desaturate : public ImageSource
{
	public:
	ImageSource_Desaturate(ImageSource *source);
	~ImageSource_Desaturate();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
