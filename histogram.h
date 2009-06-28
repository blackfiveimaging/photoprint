#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <gdk/gdkpixbuf.h>

#include "imagesource/imagesource_histogram.h"
#include "support/rwmutex.h"
#include "support/threadevent.h"

class PPHistogram : public ISHistogram, public RWMutex, public ThreadEvent
{
	public:
	PPHistogram(ThreadEventHandler &header)
		: ISHistogram(), RWMutex(), ThreadEvent(header,"HistogramBuilt")
	{
	}
	~PPHistogram()
	{
		ObtainMutex();
	}
	virtual void ReleaseMutex()
	{
		RWMutex::ReleaseMutex();
	}
	virtual GdkPixbuf *DrawHistogram(int w,int h);
	protected:
};


#endif
