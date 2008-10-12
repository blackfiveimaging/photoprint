// convkernel_gaussian.h - a class to create a gaussian convolution kernel.
// Copyright (C) 2008 by Alastair M. Robinson

#ifndef CONVKERNEL_GAUSSIAN
#define CONVKERNEL_GAUSSIAN

#include <cmath>
#include <iostream>

#include "convkernel.h"

class ConvKernel_Gaussian : public ConvKernel
{
	public:
	ConvKernel_Gaussian(float radius) : ConvKernel()
	{
		// Sanity-check the provided radius.  Any radius less than 0.5 will result in
		// a single cell kernel, containing just a "1".  Radius of zero leads to
		// a divide-by-zero later, though, so we avoid it by clamping the radius.
		if(radius<=0.1) radius=0.1;

		// Calculate the standard deviation of the gaussian distribution.
		// This is calculated such that the extreme north, south, east and west
		// cells will contain the value 0.01 - or whatever's in the log() function.
		sd=sqrt(-(radius*radius)/(2*log(0.01)));

		Initialize(int(radius+0.5)*2+1,int(radius+0.5)*2+1);
	}

	virtual ~ConvKernel_Gaussian()
	{
	}

	virtual void Initialize(int width,int height)
	{
		// Use the superclass to allocate the kernel itself;
		ConvKernel::Initialize(width,height);
		for(int y=0;y<height;++y)
		{
			int yo=y-(height/2);
			for(int x=0;x<width;++x)
			{
				int xo=x-(width/2);
				Kernel(x,y)=exp(-(xo*xo+yo*yo)/(2*sd*sd));
			}
		}
	}
	protected:
	float sd;
};

#endif
