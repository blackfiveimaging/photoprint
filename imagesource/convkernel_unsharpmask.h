// convkernel_unsharpmask.h - a class to create convolution kernel.
// Copyright (C) 2008 by Alastair M. Robinson

#ifndef CONVKERNEL_UNSHARPMASK
#define CONVKERNEL_UNSHARPMASK

#include <cmath>
#include <iostream>

#include "convkernel.h"
#include "convkernel_gaussian.h"

class ConvKernel_UnsharpMask : public ConvKernel_Gaussian
{
	public:
	ConvKernel_UnsharpMask(float radius,float amount) : ConvKernel_Gaussian(radius), amount(amount)
	{
		Normalize();
		for(int y=0;y<height;++y)
		{
			for(int x=0;x<width;++x)
			{
				Kernel(x,y)*=-amount;
			}
		}
		Kernel(width/2,height/2)+=1.0+amount;
	}

	virtual ~ConvKernel_UnsharpMask()
	{
	}

	protected:
	float amount;
};

#endif
