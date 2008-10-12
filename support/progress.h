/*
 * progress.h - base class for progress displays.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef PROGRESS_H
#define PROGRESS_H

#include <iostream>

using namespace std;

class Progress
{
	public:
	Progress() : current(0), max(1)
	{
	}
	virtual ~Progress()
	{
	}
	virtual bool DoProgress()
	{
		++current;
		return(DoProgress(current,max));
	}
	virtual bool DoProgress(int i, int maxi)
	{
		current=i;
		max=maxi;
		return(true);
	}
	virtual void SetMessage(const char *msg)
	{
	}
	protected:
	int current;
	int max;
};

#endif
