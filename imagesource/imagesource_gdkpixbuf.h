/*
 * imagesource_gdkpixbuf.h
 * 24-bit RGB and 8-bit Greyscale BMP scanline-based Loader
 * Supports Random Access
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_GDKPIXBUF_H
#define IMAGESOURCE_GDKPIXBUF_H

#include "imagesource.h"
#include <gdk/gdkpixbuf.h>
#include <fstream>

using namespace std;

class ImageSource_GdkPixbuf : public ImageSource
{
	public:
	ImageSource_GdkPixbuf(const char *filename);
	ImageSource_GdkPixbuf(GdkPixbuf *pixbuf);
	~ImageSource_GdkPixbuf();
	ISDataType *GetRow(int row);
	void Init();
	private:
	GdkPixbuf *pixbuf;
	gint bitspersample;
	gint rowstride;
	guchar *pixels;
	bool hasalpha;
};

#endif
