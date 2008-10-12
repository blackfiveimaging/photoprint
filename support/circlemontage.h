#ifndef CIRCLEMONTAGE_H
#define CIRCLEMONTAGE_H

#include "layoutrectangle.h"

class CircleMontage;

class CMSegment : public LayoutRectangle
{
	public:
	CMSegment(int x,int y,int w,int h,int xo,int yo,CircleMontage *cm,int t1,int t2);
	~CMSegment();
	private:
	float overlap;
	int radius;
	int innerradius;
	int xo;
	int yo;
	int t1;
	int t2;
	friend class ImageSource_SegmentMask;
};


class CircleMontage
{
	public:
	CircleMontage(int width,int height);
	~CircleMontage();
	void SetSegments(int segments,int angleoffset,int overlappercent);
	void SetAngleOffset(int ao);
	void SetInnerRadius(int ir);
	CMSegment *GetSegmentExtent(int seg);
	private:
	int width;
	int height;
	int xorigin;
	int yorigin;
	int segments;
	int innerradius;
	int angleoffset;
	float overlap;
	float segmentarc;
	int radius;
	friend class CMSegment;
	friend class ImageSource_SegmentMask;
};

#endif
