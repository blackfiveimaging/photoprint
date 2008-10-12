/*
 * pageextent.h - base class for Signature and GPrinter
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#ifndef PAGEEXTENT_H
#define PAGEEXTENT_H

#define DEFAULTPAGEWIDTH 595
#define DEFAULTPAGEHEIGHT 842
#define DEFAULTMARGIN 25

using namespace std;

class PageExtent
{
	public:
	PageExtent()
		: pagewidth(DEFAULTPAGEWIDTH), pageheight(DEFAULTPAGEHEIGHT),
		leftmargin(DEFAULTMARGIN), rightmargin(DEFAULTMARGIN),
		topmargin(DEFAULTMARGIN), bottommargin(DEFAULTMARGIN)
	{
	}

	PageExtent(int w,int h)
		: pagewidth(w), pageheight(h),
		leftmargin(DEFAULTMARGIN), rightmargin(DEFAULTMARGIN),
		topmargin(DEFAULTMARGIN), bottommargin(DEFAULTMARGIN)
	{
	}

	virtual ~PageExtent()
	{
	}
	
	virtual void GetImageableArea()
	{
		imageablewidth=pagewidth-(leftmargin+rightmargin);
		imageableheight=pageheight-(topmargin+bottommargin);
	}

	virtual void EqualiseMargins()
	{
		int lr=leftmargin;
		if(rightmargin>lr) lr=rightmargin;
		leftmargin=rightmargin=lr;
		int tb=topmargin;
		if(bottommargin>tb) tb=bottommargin;
		topmargin=bottommargin=tb;
		
		GetImageableArea();
	}

	virtual void SetPageExtent(PageExtent &pe)
	{
		pe.GetImageableArea();
		pagewidth=pe.pagewidth;
		pageheight=pe.pageheight;
		leftmargin=pe.leftmargin;
		rightmargin=pe.rightmargin;
		topmargin=pe.topmargin;
		bottommargin=pe.bottommargin;
		GetImageableArea();
	}

	virtual void SetMargins(int left,int right,int top,int bottom)
	{
		if((left+right)>=pagewidth)
			cerr << "Warning: margins are too wide!" << endl;
		leftmargin=left;
		rightmargin=right;

		if((top+bottom)>=pageheight)
			cerr << "Warning: margins are too tall!" << endl;
		topmargin=top;
		bottommargin=bottom;
	}

	int pagewidth,pageheight;
	int leftmargin,rightmargin,topmargin,bottommargin;
	int imageablewidth,imageableheight;
};

#endif
