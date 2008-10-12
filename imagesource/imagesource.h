/*
 * imagesource.h - base class for the efficient scanline-based
 * handling of extremely large images.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_H
#define IMAGESOURCE_H

#include <stdlib.h>

#include "imagesource_types.h"

class CMSProfile;

class ImageSource
{
	public:
	ImageSource();
	ImageSource(ImageSource *src);
	virtual ~ImageSource();
	virtual ISDataType *GetRow(int row)=0;
	void MakeRowBuffer();
	void SetResolution(double xr,double yr);
	CMSProfile *GetEmbeddedProfile();
	void SetEmbeddedProfile(CMSProfile *profile,bool assumeownership=false);
	int width,height;
	enum IS_TYPE type;
	int samplesperpixel;
	double xres,yres;
	bool randomaccess;
	protected:
	CMSProfile *embeddedprofile;
	bool embprofowned;
	int currentrow;
	ISDataType *rowbuffer;
};


#endif
