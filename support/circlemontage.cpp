#include <iostream>
#include <math.h>

#include "circlemontage.h"

using namespace std;

CMSegment::CMSegment(int x,int y,int w,int h,int xo,int yo,CircleMontage *cm,int t1,int t2)
	: LayoutRectangle(x,y,w,h), overlap(cm->overlap), radius(cm->radius), innerradius(cm->innerradius),
	xo(xo), yo(yo), t1(t1), t2(t2)
{
}


CMSegment::~CMSegment()
{
}


void CircleMontage::SetSegments(int segments,int angleoffset,int overlappercent)
{
	this->segments=segments;
	segmentarc=360.0/segments;
	overlap=(segmentarc*overlappercent)/100.0;
	this->angleoffset=int(angleoffset-segmentarc);
}


CMSegment *CircleMontage::GetSegmentExtent(int segment)
{
	float degperrad=360.0/(2*M_PI);
	float t1=angleoffset+segment*segmentarc-overlap;
	float t2=angleoffset+(segment+1)*segmentarc+overlap;
	if(t2>=360.0)
	{
		t1-=360.0;
		t2-=360.0;
	}
	float tx1=radius*sin(t1/degperrad);
	float ty1=-radius*cos(t1/degperrad);
//	float tx2=radius2*sin(t1/degperrad);
//	float ty2=-radius2*cos(t1/degperrad);
	float tx2=0.0;
	float ty2=0.0;
	float tx3=radius*sin(t2/degperrad);
	float ty3=-radius*cos(t2/degperrad);
//	float tx4=radius2*sin(t2/degperrad);
//	float ty4=-radius2*cos(t2/degperrad);
	float tx4=0.0;
	float ty4=0.0;

	float x1,x2,y1,y2;
	x1=tx1;
	if(tx2<x1) x1=tx2;
	if(tx3<x1) x1=tx3;
	if(tx4<x1) x1=tx4;

	y1=ty1;
	if(ty2<y1) y1=ty2;
	if(ty3<y1) y1=ty3;
	if(ty4<y1) y1=ty4;

	x2=tx1;
	if(tx2>x2) x2=tx2;
	if(tx3>x2) x2=tx3;
	if(tx4>x2) x2=tx4;

	y2=ty1;
	if(ty2>y2) y2=ty2;
	if(ty3>y2) y2=ty3;
	if(ty4>y2) y2=ty4;

	if(t1<0.0 && t2>0.0)
		y1=-radius;
	if(t1<90.0 && t2>90.0)
		x2=radius;
	if(t1<-270.0 && t2>-270.0)
		x2=radius;
	if(t1<180.0 && t2>180.0)
		y2=radius;
	if(t1<-180.0 && t2>-180.0)
		y2=radius;
	if(t1<270.0 && t2>270.0)
		x1=-radius;
	if(t1<-90.0 && t2>-90.0)
		x1=-radius;

	return(new CMSegment(int(xorigin+x1),int(yorigin+y1),int(x2-x1),int(y2-y1),
		int(-x1),int(-y1),this,int(t1),int(t2)));
}


CircleMontage::~CircleMontage()
{
}


CircleMontage::CircleMontage(int width,int height)
	: width(width), height(height), xorigin(width/2), yorigin(height/2), innerradius(0)
{
	radius=width/2;
	if((height/2)<radius)
		radius=height/2;
	SetSegments(2,0,33);
}


void CircleMontage::SetInnerRadius(int ir)
{
	innerradius=ir;
}
