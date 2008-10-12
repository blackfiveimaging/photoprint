/*
 * imagesource_pnm.cpp - ImageSource loader for PNM files.
 *
 * Supports high bit depths,
 * RGB data.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if defined HAVE_LIBPNM || defined HAVE_LIBNETPBM

#include "imagesource_pnm.h"

using namespace std;

ImageSource_PNM::~ImageSource_PNM()
{
	if(tuplerow)
		free(tuplerow);
	
	if(file)
		fclose(file);
}


ISDataType *ImageSource_PNM::GetRow(int row)
{
	ISDataType *dst;

	if(currentrow==row)
		return(rowbuffer);

	if(row<currentrow)
		throw "Random access not yet supported in PNM files!";

	for(;currentrow<row;++currentrow)
		pnm_readpamrow(&header,tuplerow);
	dst=rowbuffer;
	
	switch(samplesperpixel)
	{
		int x;
		case 3:
			for(x=0;x<width;++x)
			{
				sample s=tuplerow[x][0];
				*dst++=(s*IS_SAMPLEMAX)/header.maxval;
				s=tuplerow[x][1];
				*dst++=(s*IS_SAMPLEMAX)/header.maxval;
				s=tuplerow[x][2];
				*dst++=(s*IS_SAMPLEMAX)/header.maxval;
			}
			break;
		case 1:
			switch(header.maxval)
			{
				case 255:
					for(x=0;x<width;++x)
					{
						*dst++=EIGHTTOIS(tuplerow[x][0]);
					}
					break;
				case 65535:
					for(x=0;x<width;++x)
					{
						*dst++=tuplerow[x][0];
					}
					break;
			}
			break;
	}
	return(rowbuffer);
}

ImageSource_PNM::ImageSource_PNM(const char *filename)
{
	char *argv[]={"PNMLoader",0};
	int argc=1;

	pnm_init(&argc,argv);

	if(!(file=fopen(filename,"rb")))
		throw "Can't open file";

	cerr << "Attempting to read PNM file..." << endl;

	pnm_readpaminit(file, &header, sizeof(struct pam));

	width=header.width;
	height=header.height;
	xres=72;
	yres=72;
	randomaccess=false;
	
	switch(header.depth)
	{
		case 1:
			type=IS_TYPE_GREY;
			samplesperpixel=1;
			break;
		case 3:
			type=IS_TYPE_RGB;
			samplesperpixel=3;
			break;
		default:
			throw "PNM header has unsupported depth";
			break;
	}

	tuplerow = pnm_allocpamrow(&header);

	MakeRowBuffer();

	embeddedprofile=NULL;
}

#endif
