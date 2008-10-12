// convkernel_gaussian.h - a class to create a gaussian convolution kernel.
// Copyright (C) 2008 by Alastair M. Robinson

#ifndef CONVKERNEL_GAUSSIAN_1D
#define CONVKERNEL_GAUSSIAN_1D

#include <cmath>
#include <iostream>

#include "convkernel.h"

class ConvKernel_Gaussian_1D : public ConvKernel
{
	public:
	ConvKernel_Gaussian_1D(float radius) : ConvKernel()
	{
		// Sanity-check the provided radius.  Any radius less than 0.5 will result in
		// a single cell kernel, containing just a "1".  Radius of zero leads to
		// a divide-by-zero later, though, so we avoid it by clamping the radius.
		if(radius<=0.1) radius=0.1;

		// Calculate the standard deviation of the gaussian distribution.
		// This is calculated such that the extreme north, south, east and west
		// cells will contain the value 0.01 - or whatever's in the log() function.
		sd=sqrt(-(radius*radius)/(2*log(0.01)));

		Initialize(int(radius+0.5)*2+1);
	}

	virtual ~ConvKernel_Gaussian_1D()
	{
	}

	virtual void Initialize(int width)
	{
		// Use the superclass to allocate the kernel itself;
		ConvKernel::Initialize(width,1);
		for(int x=0;x<width;++x)
		{
			int xo=x-(width/2);
			Kernel(x,0)=exp(-(xo*xo)/(2*sd*sd));
		}
	}
	protected:
	float sd;
};

#endif
