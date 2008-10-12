#include <iostream>
#include <stdio.h>

using namespace std;

#include "layoutrectangle.h"


LayoutRectangle::LayoutRectangle(int x,int y,int w,int h) : x(x), y(y), w(w), h(h)
{
}


LayoutRectangle::LayoutRectangle(int w,int h) : x(0), y(0), w(w), h(h)
{
}


LayoutRectangle::~LayoutRectangle()
{
}


LayoutRectangle *LayoutRectangle::Intersection(LayoutRectangle &r)
{
	LayoutRectangle *result=NULL;
	int x2=x+w;
	int y2=y+h;
	int x4=r.x+r.w;
	int y4=r.y+r.h;
	int nx,ny,nx2,ny2,nw,nh;

	if(x>r.x)
		nx=x;
	else
		nx=r.x;

	if(y>r.y)
		ny=y;
	else
		ny=r.y;

	if(x2<x4)
		nx2=x2;
	else
		nx2=x4;

	if(y2<y4)
		ny2=y2;
	else
		ny2=y4;

	nw=nx2-nx;
	nh=ny2-ny;

	if((nw>0) && (nh>0))
		result=new LayoutRectangle(nx,ny,nw,nh);

	return(result);
}


LayoutRectangle *LayoutRectangle::UnionBoundary(LayoutRectangle &r)
{
	LayoutRectangle *result=NULL;
	int x2=x+w;
	int y2=y+h;
	int x4=r.x+r.w;
	int y4=r.y+r.h;
	int nx,ny,nx2,ny2,nw,nh;

	if(x>r.x)
		nx=r.x;
	else
		nx=x;

	if(y>r.y)
		ny=r.y;
	else
		ny=y;

	if(x2<x4)
		nx2=x4;
	else
		nx2=x2;

	if(y2<y4)
		ny2=y4;
	else
		ny2=y2;

	nw=nx2-nx;
	nh=ny2-ny;

	if((nw>0) && (nh>0))
		result=new LayoutRectangle(nx,ny,nw,nh);

	return(result);
}


RectFit *LayoutRectangle::Fit(LayoutRectangle &r,bool allow_cropping,PP_ROTATION rotation,
	LayoutRectangle_Alignment halign,LayoutRectangle_Alignment valign)
{
	RectFit *fit=new RectFit;
	int nw,nh;

	switch(rotation)
	{
		case PP_ROTATION_NONE:
			fit->rotation=0;
			fit->width=w;
			fit->height=h;
			break;
		case PP_ROTATION_90:
			fit->rotation=90;
			fit->width=h;
			fit->height=w;
			break;
		case PP_ROTATION_180:
			fit->rotation=180;
			fit->width=w;
			fit->height=h;
			break;
		case PP_ROTATION_270:
			fit->rotation=270;
			fit->width=h;
			fit->height=w;
			break;
		case PP_ROTATION_AUTO:
			if((w>h)^(r.w>r.h))
			{
				fit->rotation=90;
				fit->width=h;
				fit->height=w;
			}
			else
			{
				fit->rotation=0;
				fit->width=w;
				fit->height=h;
			}
			break;
	}

	nw=r.w;
	fit->scale=r.w; fit->scale/=fit->width;
	nh=int(fit->scale*fit->height);

	if(allow_cropping)
	{
		if(nh<r.h)
		{
			nh=r.h;
			fit->scale=r.h; fit->scale/=fit->height;
			nw=int(fit->scale*fit->width);
		}
		fit->width=nw;
		fit->height=nh;
		fit->xpos=r.x;
		fit->ypos=r.y;
		
		fit->xoffset=((nw-r.w)*halign)/LAYOUT_RECTANGLE_PANNING_MAX;
		fit->yoffset=((nh-r.h)*valign)/LAYOUT_RECTANGLE_PANNING_MAX;

#if 0
		switch(halign)
		{
			case START:
				fit->xoffset=0;
				break;
			case CENTRE:
				fit->xoffset=(nw-r.w)/2;
				break;
			case END:
				fit->xoffset=(nw-r.w);
				break;
		}
		switch(valign)
		{
			case START:
				fit->yoffset=0;
				break;
			case CENTRE:
				fit->yoffset=(nh-r.h)/2;
				break;
			case END:
				fit->yoffset=(nh-r.h);
				break;
		}
#endif
	}
	else
	{
		if(nh>r.h)
		{
			nh=r.h;
			fit->scale=r.h; fit->scale/=fit->height;
			nw=int(fit->width*fit->scale);
		}
		fit->width=nw;
		fit->height=nh;
		fit->xpos=r.x+(r.w-nw)/2;
		fit->ypos=r.y+(r.h-nh)/2;
		fit->xoffset=0;
		fit->yoffset=0;	
	}
	return(fit);
}


void LayoutRectangle::Scale(double scale)
{
	x=int(x*scale+0.5);
	y=int(y*scale+0.5);
	w=int(w*scale+0.5);
	h=int(h*scale+0.5);
}
