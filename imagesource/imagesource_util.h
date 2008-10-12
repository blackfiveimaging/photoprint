#ifndef IMAGESOURCE_UTIL_H
#define IMAGESOURCE_UTIL_H

#include "imagesource.h"

enum IS_ScalingQuality
{
	IS_SCALING_AUTOMATIC,
	IS_SCALING_NEARESTNEIGHBOUR,
	IS_SCALING_BILINEAR,
	IS_SCALING_LANCZOSSINC,
	IS_SCALING_MAX,
	IS_SCALING_DOWNSAMPLE	// An extra entry used for high-quality image reduction.
};

struct IS_ScalingQualityDescription
{
	const char *Name;
	const char *Description;
};


ImageSource *ISLoadImage(const char *filename);
ImageSource *ISScaleImageByResolution(ImageSource *source,double xres,double yres,IS_ScalingQuality quality=IS_SCALING_AUTOMATIC);
ImageSource *ISScaleImageBySize(ImageSource *source,int w,int h,IS_ScalingQuality quality=IS_SCALING_AUTOMATIC);
const IS_ScalingQualityDescription *DescribeScalingQuality(IS_ScalingQuality quality);

#endif
