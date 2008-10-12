/*
 * imagesource_bmp.h
 * 24-bit RGB and 8-bit Greyscale BMP scanline-based Loader
 * Supports Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_BMP_H
#define IMAGESOURCE_BMP_H

#include "imagesource.h"
#include <fstream>

using namespace std;

class IS_BMPStrip;

class ImageSource_BMP : public ImageSource
{
	public:
	ImageSource_BMP(const char *filename);
	~ImageSource_BMP();
	ISDataType *GetRow(int row);
	private:
	IS_BMPStrip *GetStrip(int row);
	unsigned long GetValue(char *c,int l);
	int resunit;
	fstream file;
	int cmapbytes;
	int cmapentries;
	int bytesperrow;
	long stripsize;
	int imagestart;
	int palette[256][3];
	IS_BMPStrip *strips;
	friend class IS_BMPStrip;
};

#endif
