/*
 * print.cpp - encapsulates printing via GIMP-Print/GutenPrint
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef PRINT_H
#define PRINT_H

#include "support/configdb.h"
#include "support/pageextent.h"
#include "support/consumer.h"
#include "printoutput.h"
#include "support/progress.h"
#include <gutenprint/gutenprint.h>

#include "gprintersettings.h"


class ImageSource; // Forward Declaration

class GPrinter : public GPrinterSettings
{
	public:
	GPrinter(PrintOutput &output,ConfigFile *inf,const char *section);
	~GPrinter();
	void Print(ImageSource *source,int xpos,int ypos);
	void Help();
	void SetProgress(Progress *p);
	void GetImageableArea();
	void GetSizeLimits(int &minw,int &maxw,int &minh,int &maxh);
	void SetCustomWidth(int w);
	void SetCustomHeight(int h);
	private:
	stp_image_status_t GetRow(int row,unsigned char *data);

	void get_dimensions();
	void custom_get_dimensions();
	void print_dimensions();

	ImageSource *source;
	Consumer *consumer;
	
	int xpos,ypos;

	int ptwidth, ptheight;
	int pixelwidth, pixelheight;

	int paperleft;
	int papertop;
	
	int firstrow;
	int firstpixel;

	int leftbleed;
	int rightbleed;
	int topbleed;
	int bottombleed;
	
	int minwidth;
	int maxwidth;
	int minheight;
	int maxheight;

	const stp_printer_t *the_printer;

	Progress *progress;

	/* Static members and stubs */
	static stp_image_t stpImage;
	static bool writeerror;
	static void writefunc(void *file, const char *buf, size_t bytes);
	static void Image_init(stp_image_t *img);
	static stp_image_status_t GetRowStub(stp_image_t *img, unsigned char *data,
		size_t byte_limit, int row);
	static int Image_width(stp_image_t *img);
	static int Image_height(stp_image_t *img);
	static const char *Image_get_appname(struct stp_image *image);
};


#endif
