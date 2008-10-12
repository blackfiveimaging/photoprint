/*
 * jpegsave.cpp - class for saving an ImageSource to disk as a JPEG file.
 *
 * Copyright (c) 2007 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 *
 */


#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <sys/stat.h>

#include "lcmswrapper.h"
#include "imagesource_flatten.h"

#include "iccjpeg.h"
#include "jpegsave.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)

using namespace std;


struct JPEGSaver_ErrManager
{
	struct jpeg_error_mgr std;
	FILE *file;
};


static void isjpeg_error_exit (j_common_ptr cinfo)
{
	char buffer[JMSG_LENGTH_MAX];
	JPEGSaver_ErrManager *myerr = (JPEGSaver_ErrManager *) cinfo->err;
	cinfo->err->output_message(cinfo);
	cinfo->err->format_message(cinfo,buffer);
	cerr << buffer << endl;
	fclose(myerr->file);
	exit(1);
}


JPEGSaver::~JPEGSaver()
{
	if(err)
	{
		if(err->file)
			fclose(err->file);
		delete err;
	}
	jpeg_destroy_compress((jpeg_compress_struct *)cinfo);
	delete cinfo;
	
	if(tmpbuffer)
		free(tmpbuffer);
}


void JPEGSaver::Save()
{	
	if(err->file)
	{
		int i;
		unsigned char *dst;
		ISDataType *src;

		for(int row=0;row<height;++row)
		{
			if(progress && !(row&31))
			{
				if(!progress->DoProgress(row,height))
					return;
			}

			dst=tmpbuffer;
				
			src=imagesource->GetRow(row);

			switch(imagesource->type)
			{
				case IS_TYPE_RGB:
					for(i=0;i<bytesperrow;++i)
					{
						unsigned int t;
						t=int(src[i]);
						t=ISTOEIGHT(t);
						if(t>255) t=255;
						if(t<0) t=0;
						dst[i]=t;
					}
					break;
				default:
					for(i=0;i<bytesperrow;++i)
					{
						unsigned int t;
						t=IS_SAMPLEMAX-src[i];
						t=ISTOEIGHT(t);
						if(t>255) t=255;
						if(t<0) t=0;
						dst[i]=t;
					}
					break;
			}
			JSAMPROW rowptr[1]={tmpbuffer};
			jpeg_write_scanlines(cinfo, rowptr, 1);
		}
		if(progress)
			progress->DoProgress(height,height);
		jpeg_finish_compress(cinfo);
		jpeg_destroy_compress(cinfo);
	}
}


void JPEGSaver::SetProgress(Progress *progress)
{
	this->progress=progress;
}


void JPEGSaver::EmbedProfile(CMSProfile *profile)
{
	if(!profile)
		return;
	FILE* f;
	size_t size, EmbedLen;
	JOCTET *EmbedBuffer;

	const char *fn=profile->GetFilename();
	if(!fn)
		return;

	if(!(f = fopen(fn, "rb")))
		return;
	
	fseek(f,0,SEEK_END);
	size=ftell(f);
	fseek(f,0,SEEK_SET);

	cerr << "Profile " << fn << " is " << size << "bytes." << endl;

	EmbedBuffer = (JOCTET *) malloc(size + 1);
	EmbedLen = fread(EmbedBuffer, 1, size, f);
	fclose(f);
	EmbedBuffer[EmbedLen] = 0;
	
	write_icc_profile(cinfo,EmbedBuffer,EmbedLen);
	free(EmbedBuffer);
}


JPEGSaver::JPEGSaver(const char *filename,struct ImageSource *is,int compression)
	: imagesource(is), tmpbuffer(NULL), progress(NULL)
{
	if(STRIP_ALPHA(is->type)==IS_TYPE_BW)
		throw _("JPEG Saver only supports greyscale and colour images!");

	if(STRIP_ALPHA(is->type)==IS_TYPE_CMYK)
		throw _("Saving CMYK JPEGs not (yet) supported");

	if(HAS_ALPHA(is->type))
		is=new ImageSource_Flatten(is);

	this->width=is->width;
	this->height=is->height;

	this->xres=is->xres;
	this->yres=is->yres;

	cinfo=new jpeg_compress_struct;
	err=new JPEGSaver_ErrManager;
	memset(cinfo,sizeof(jpeg_compress_struct),0);
	memset(err,sizeof(JPEGSaver_ErrManager),0);
	cinfo->err = jpeg_std_error(&err->std);
	err->std.error_exit = isjpeg_error_exit;

	if((err->file = fopen(filename,"wb")) == NULL)
		throw _("Can't open file for saving");

	cerr << "File " << filename << " opened" << endl;

	jpeg_create_compress(cinfo);
	jpeg_stdio_dest(cinfo, err->file);

	cinfo->image_width=is->width;
	cinfo->image_height=is->height;

	switch(is->type)
	{
		case IS_TYPE_RGB:
			cinfo->input_components=3;
			cinfo->in_color_space = JCS_RGB;
			jpeg_set_defaults(cinfo);
			break;
		case IS_TYPE_GREY:
			cinfo->input_components=1;
			cinfo->in_color_space = JCS_GRAYSCALE;
			jpeg_set_defaults(cinfo);
			jpeg_set_colorspace(cinfo,JCS_GRAYSCALE);
			break;
		case IS_TYPE_CMYK:
			throw _("Saving CMYK JPEGs not (yet) supported");
			break;
		default:
			throw _("JPEG Saver can currently only save RGB or Greyscale images.");
			break;
	}
	jpeg_set_quality(cinfo,compression,TRUE);
	jpeg_start_compress(cinfo,TRUE);

	if(is->GetEmbeddedProfile())
		EmbedProfile(is->GetEmbeddedProfile());

	bytesperrow = width*is->samplesperpixel;

	if(!(tmpbuffer=(unsigned char *)malloc(bytesperrow)))
		throw "No memory for tmpbuffer";
}
