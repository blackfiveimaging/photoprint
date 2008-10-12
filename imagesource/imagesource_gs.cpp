/*
 * imagesource_gs.cpp
 * 24-bit RGB and 8-bit Greyscale Loader for PS / PDF
 * Delegates actual loading to ImageSource_TIFF
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imagesource_gs.h"

using namespace std;


ImageSource_GS::~ImageSource_GS()
{
	if(tiffname)
	{
		remove(tiffname);
		free(tiffname);
	}
}


#define OFFSET(pb, x, y) ((x) * (gdk_pixbuf_get_n_channels(pb)) + (y) * gdk_pixbuf_get_rowstride(pb))

ISDataType *ImageSource_GS::GetRow(int row)
{
	if(source)
		return(source->GetRow(row));
	else
		return(NULL);
}


ImageSource_GS::ImageSource_GS(const char *filename,int resolution) : ImageSource(), tiffname(NULL)
{
	// Create temporary name
	tiffname=tempnam(NULL,"phopr");
	
	// Build GS Command line
	const char *gscmd="gs -r%dx%d -sDEVICE=tiff24nc -sOutputFile=%s -dFirstPage=1 -dLastPage=1 -dBATCH -dNOPAUSE %s";

	if(resolution>2400)
		resolution=2400;

	int l=strlen(gscmd)+strlen(tiffname)+strlen(filename)+8;
	char *cmd=(char *)malloc(l);
	
	if(!cmd)
		throw "Can't create GhostScript command line!";

	snprintf(cmd,l,gscmd,resolution,resolution,tiffname,filename);
	
	// Execute command
	system(cmd);
	
	// Free command
	free(cmd);

	// Create imagesource from temporary TIFF file
	source=new ImageSource_TIFF(tiffname);

	// Copy details from source to this.

	width=source->width;
	height=source->height;
	xres=source->xres;
	yres=source->yres;
//	embeddedprofile=source->embeddedprofile;  // FIXME: Steal this?  Copy it?
	type=source->type;
	samplesperpixel=source->samplesperpixel;
	randomaccess=source->randomaccess;
}

