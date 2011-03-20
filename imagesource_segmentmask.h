/*
 * imagesource_segmentmask.h
 * Supports Random Access
 *
 * Copyright (c) 2005 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef IMAGESOURCE_SEGMENTMASK_H
#define IMAGESOURCE_SEGMENTMASK_H

#include "imagesource.h"

class CMSegment;
class ImageSource_SegmentMask : public ImageSource
{
	public:
	ImageSource_SegmentMask(CMSegment *seg,bool fade);
	~ImageSource_SegmentMask();
	ISDataType *GetRow(int row);
	private:
	CMSegment *segment;
	bool fade;
};

#endif
