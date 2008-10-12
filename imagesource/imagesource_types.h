#ifndef IMAGESOURCE_TYPES_H
#define IMAGESOURCE_TYPES_H

typedef unsigned short ISDataType;
#define IS_SAMPLEMAX 65535
#define EIGHTTOIS(x) (((x) << 8) | (x))
#define ISTOEIGHT(x) (((x) >> 8) & 0xff )
//#define ISTOEIGHT(x) ((((x) * 65281 + 8388608) >> 24) & 0xff)

/* Note: 0 is Black for RGB images, but White for Greyscale and CMYK images */

enum IS_TYPE {
	IS_TYPE_NULL=0,
	IS_TYPE_BW,
	IS_TYPE_GREY,
	IS_TYPE_RGB,
	IS_TYPE_CMYK,
	IS_TYPE_NULLA=8,
	IS_TYPE_BWA,
	IS_TYPE_GREYA,
	IS_TYPE_RGBA,
	IS_TYPE_CMYKA
};

#define IS_TYPE_ALPHA 8
#define IS_MAX_SAMPLESPERPIXEL 5
#define STRIP_ALPHA(x) IS_TYPE(((x)&~IS_TYPE_ALPHA))
#define HAS_ALPHA(x) ((x)&IS_TYPE_ALPHA)

#endif
