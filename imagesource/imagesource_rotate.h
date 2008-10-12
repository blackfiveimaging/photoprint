/*
 * imagesource_rotate.h - filter to rotate and image through
 * 0, 90, 180 or 270 degrees.
 * supports random access, even if source image doesn't.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_ROTATE_H
#define IMAGESOURCE_ROTATE_H

#include "imagesource.h"

struct ImageSource *ImageSource_Rotate_New(struct ImageSource *source,int rotation,int spanrows);

class ImageSource_Rotate : public ImageSource
{
	public:
	ImageSource_Rotate(ImageSource *source,int rotation,int spanrows=1024);
	~ImageSource_Rotate();
	ISDataType *GetRow(int row);
	private:
	ImageSource *source;
	int rotation;
	int spanfirstrow;
	int spanrows;
	int samplesperrow;
	ISDataType *spanbuffer;
};

#endif
