/*
 * imagesource_promote.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_PROMOTE_H
#define IMAGESOURCE_PROMOTE_H

#include "imagesource.h"

class ImageSource_Promote : public ImageSource
{
	public:
	ImageSource_Promote(ImageSource *source,IS_TYPE type);
	~ImageSource_Promote();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
};

#endif
