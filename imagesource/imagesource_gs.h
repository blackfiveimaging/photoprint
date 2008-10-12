/*
 * imagesource_gs.h
 * 24-bit RGB and 8-bit Greyscale Loader for PS / PDF
 * Delegates actual loading to ImageSource_TIFF
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GS_H
#define IMAGESOURCE_GS_H

#define IMAGESOURCE_GS_DEFAULT_RESOLUTION 360

#include "imagesource.h"
#include "imagesource_tiff.h"
#include <fstream>

using namespace std;

class ImageSource_GS : public ImageSource
{
	public:
	ImageSource_GS(const char *filename,int resolution);
	~ImageSource_GS();
	ISDataType *GetRow(int row);
	private:
	char *tiffname;
	ImageSource *source;
};

#endif
