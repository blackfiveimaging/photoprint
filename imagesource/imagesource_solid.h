/*
 * imagesource_solid.h
 *
 * Supports RGB and CMYK data
 * Supports random access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_SOLID_H
#define IMAGESOURCE_SOLID_H

#include "imagesource.h"

class ImageSource_Solid : public ImageSource
{
	public:
	ImageSource_Solid(IS_TYPE type,int width,int height,ISDataType *solid=NULL);
	~ImageSource_Solid();
	ISDataType *GetRow(int row);
	private:	
	ISDataType solid[IS_MAX_SAMPLESPERPIXEL];
};

#endif
