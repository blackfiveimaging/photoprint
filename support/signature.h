/*
 * signature.h - class to handle coordinate calculations for n-up rectangular layouts
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include "pageextent.h"
#include "layoutrectangle.h"

#define DEFAULTGUTTER 15


class LayoutRectangle;

class Signature : public virtual PageExtent
{
	public:
	Signature(int rows=1,int columns=1);
	Signature(PageExtent &extent,int rows=1,int columns=1);
	~Signature()
	{
	}
	void SetPageExtent(PageExtent &pe);
	void SetPaperSize(int width,int height);
	void SetMargins(int left,int right,int top,int bottom);
	void SetGutters(int hgutter,int vgutter);
	void SetColumns(int columns);
	void SetRows(int rows);
	int GetColumns()
	{
		return columns;
	}
	int GetRows()
	{
		return rows;
	}
	int GetHGutter()
	{
		return hgutter;
	}
	int GetVGutter()
	{
		return vgutter;
	}
	LayoutRectangle *GetLayoutRectangle(int row,int column);
	void EqualiseMargins();
	void ReCalc();
	int ColumnAt(int xpos);
	int RowAt(int ypos);
	private:
	int hgutter,vgutter;
	int rows,columns;
	float celwidth,celheight;
};

#endif
