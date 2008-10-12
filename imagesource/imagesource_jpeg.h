/*
 * imagesource_jpeg.h
 * 24-bit RGB and 8-bit Greyscale JPEG scanline-based Loader
 * Doesn't support Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_JPEG_H
#define IMAGESOURCE_JPEG_H

#include "imagesource.h"
#include <stdio.h>

using namespace std;

class ImageSource_JPEG : public ImageSource
{
	public:
	ImageSource_JPEG(const char *filename);
	~ImageSource_JPEG();
	ISDataType *GetRow(int row);
	private:
	FILE *file;
	struct jpeg_decompress_struct *cinfo;
	unsigned char *tmprow;
	struct ImageSource_JPEG_ErrManager *err;
	char *iccprofbuffer;
	bool started;
};

#endif
