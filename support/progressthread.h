#ifndef PROGRESSTHREAD_H
#define PROGRESSTHREAD_H

#include "thread.h"
#include "progress.h"

class ProgressThread : public Progress
{
	public:
	ProgressThread(Thread &t) : Progress(), thread(t)
	{
	}
	virtual ~ProgressThread()
	{
	}
	virtual bool DoProgress()
	{
		return(!thread.TestBreak());
	}
	virtual bool DoProgress(int i, int maxi)
	{
		return(!thread.TestBreak());
	}
	protected:
	Thread &thread;
};


#endif

