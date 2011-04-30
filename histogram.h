#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <gdk/gdkpixbuf.h>

#include "imagesource/imagesource_histogram.h"
#include "rwmutex.h"
#include "threadevent.h"
#include "debug.h"

class PPHistogram : public ISHistogram, public RWMutex
{
	public:
	PPHistogram(ThreadEventHandler &header)
		: ISHistogram(), RWMutex(), event(header,"HistogramBuilt")
	{
	}
	~PPHistogram()
	{
		Debug[TRACE] << "In PPHistogram's destructor" << std::endl;
		ObtainMutex();
		Debug[TRACE] << "PPHistogram's destructor completed" << std::endl;
	}
	virtual void ReleaseMutex()
	{
		RWMutex::ReleaseMutex();
	}
	virtual GdkPixbuf *DrawHistogram(int w,int h);
	ThreadEvent event;
	protected:
};


#endif
