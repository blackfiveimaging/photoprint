/*
 * imagesource_tiff.h - ImageSource loader for TIFF files.
 *
 * Supports 8 and 16 bit,
 * RGB, CMYK and Grey data
 * Indexed and monochrome files are currently converted to grey.
 * Supports random access (to the extent that libtiff does, i.e.
 * depends on the format of the file).
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * TODO: Support tile-based images
 * Test with 16-bit CMYK, Grey
 * Add support for Lab
 * Add indexed->RGB conversion
 *
 */

#ifndef IMAGESOURCE_TIFF_H
#define IMAGESOURCE_TIFF_H

#include <tiffio.h>
#include "imagesource.h"

class IS_TIFFStrip;

class ImageSource_TIFF : public ImageSource
{
	public:
	ImageSource_TIFF(const char *filename);
	~ImageSource_TIFF();
	ISDataType *GetRow(int row);
	private:
	IS_TIFFStrip *GetStrip(int row);
	IS_TIFFStrip *LoadStrip(int row);
	int CountTIFFDirs(const char *filename,int &largestdir);
	int resunit;
	TIFF *file;
	int bps,spr;
	int source_spp;
	int photometric;
	long stripsize;
	int stripheight;
	int stripcount;
	int filerow;
	int greypalette[256];
	IS_TIFFStrip *strips;
	friend class IS_TIFFStrip;
};

#endif
