#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include "imagesource/imagesource_histogram.h"
#include "support/rwmutex.h"

class PPHistogram : public ISHistogram, RWMutex
{
	public:
	PPHistogram() : ISHistogram()
	{
	}
	~PPHistogram()
	{
	}
	protected:
};

#endif
