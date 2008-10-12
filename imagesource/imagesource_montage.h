/*
 * imagesource_montage.h
 * Composites multiple images into a single image.
 * Supports Random Access if and only if all source images also support it.
 *
 * Copyright (c) 2004, 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_MONTAGE_H
#define IMAGESOURCE_MONTAGE_H

#include "imagesource.h"
#include "imagesource_util.h"

class ISMontage_Component;

class ImageSource_Montage : public ImageSource
{
	public:
	ImageSource_Montage(IS_TYPE type,int res=360);
	~ImageSource_Montage();
	virtual void Add(ImageSource *is,int xpos,int ypos);
	ISDataType *GetRow(int row);
	protected:
	ISMontage_Component *first;
	friend class ISMontage_Component;
};

#endif
