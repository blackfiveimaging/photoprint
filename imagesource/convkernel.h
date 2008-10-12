// convkernel.h - a virtual superclass defining a convolution kernel interface.
// Copyright (C) 2008 by Alastair M. Robinson

#ifndef CONVKERNEL
#define CONVKERNEL
#include <iostream>

class ConvKernel
{
	public:
	ConvKernel() : width(0), height(0), kernel(NULL)
	{
	}
	ConvKernel(int width,int height) : kernel(NULL)
	{
		Initialize(width,height);
	}

	// Initialize function allocates and clears the kernel
	// and sets the width and height fields.  Will typically
	// be called by subclasses' constructors.
	// This model allows the subclass to calculate the width
	// and height from some other parameter, such as radius for
	// a gaussian kernel, before calling this function.
	virtual void Initialize(int width,int height)
	{
		this->width=width;
		this->height=height;
		kernel=new float[width*height];
		for(int y=0;y<height;++y)
		{
			for(int x=0;x<width;++x)
			{
				Kernel(x,y)=0.0;
			}
		}
	}
	virtual ~ConvKernel()
	{
		if(kernel)
			delete[](kernel);
	}

	// Simple accessors;
//	virtual float *GetKernel()
//	{
//		return(kernel);
//	}
	virtual int GetWidth()
	{
		return(width);
	}
	virtual int GetHeight()
	{
		return(height);
	}

	// Sums the kernel, then divides each cell by the sum,
	// so that the coefficients add up to 1.
	virtual void Normalize()
	{
		float total=0;
		for(int y=0;y<height;++y)
		{
			for(int x=0;x<width;++x)
			{
				total+=Kernel(x,y);
			}
		}
		if(total>0.0)
		{
			for(int y=0;y<height;++y)
			{
				for(int x=0;x<width;++x)
				{
					Kernel(x,y)/=total;
				}
			}
		}
	}
	
	// Accessor function - returns a reference to a particular cell.
	// Inlined for speed.
	inline float &Kernel(int x,int y)
	{
		return(kernel[y*width+x]);
	}
	
	// Function to dump the contents of a kernel to an output stream
	friend std::ostream& operator<<(std::ostream &s,ConvKernel &ck)
	{
		for(int y=0;y<ck.height;++y)
		{
			for(int x=0;x<ck.width;++x)
			{
				std::cerr << ck.kernel[y*ck.width+x] << "    ";
			}
			std::cerr << std::endl;
		}
		return(s);
	}
	protected:
	int width,height;
	float *kernel;	
};

#endif
