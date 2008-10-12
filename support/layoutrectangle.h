#ifndef LAYOUTRECTANGLE_H
#define LAYOUTRECTANGLE_H

enum PP_ROTATION
{
	PP_ROTATION_AUTO,
	PP_ROTATION_NONE,
	PP_ROTATION_90,
	PP_ROTATION_180,
	PP_ROTATION_270,
};


struct RectFit
{
	int xpos,ypos;
	int xoffset,yoffset;
	int width,height;
	int rotation;
	double scale;
	void Dump()
	{
		printf("RectFit: pos: (%d,%d), offset: (%d, %d)\n",xpos,ypos,xoffset,yoffset);
		printf(" size: (%d,%d), rotation: %d, scale: %lf\n",width,height,rotation,scale);
	}
};

#define LAYOUT_RECTANGLE_PANNING_MAX 100
enum LayoutRectangle_Alignment {START=0,CENTRE=LAYOUT_RECTANGLE_PANNING_MAX/2,END=LAYOUT_RECTANGLE_PANNING_MAX};

class LayoutRectangle
{
	public:
	LayoutRectangle(int x,int y,int w,int h);
	LayoutRectangle(int w,int h);
	LayoutRectangle(LayoutRectangle &r);
	virtual ~LayoutRectangle();
	virtual LayoutRectangle *Intersection(LayoutRectangle &r);
	virtual LayoutRectangle *UnionBoundary(LayoutRectangle &r);
	virtual RectFit *Fit(LayoutRectangle &r,bool allow_cropping,PP_ROTATION rotation,
		LayoutRectangle_Alignment halign,LayoutRectangle_Alignment valign);
	virtual void Scale(double scale);
	void Dump()
	{
		printf("LayoutRectangle at: (%d,%d), size: (%d, %d)\n",x,y,w,h);
	}
	int x,y;
	int w,h;
};


#endif
