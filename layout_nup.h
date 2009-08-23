#ifndef LAYOUT_NUP_H
#define LAYOUT_NUP_H

#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "support/signature.h"
#include "imagesource/imagesource.h"
#include "gp_cppsupport/gprinter.h"

#include "layoutdb.h"
#include "layout.h"

class Layout_NUp_ImageInfo;
class PhotoPrint_State;

class Layout_NUp : public Layout, public Signature
{
	public:
	Layout_NUp(PhotoPrint_State &state,Layout *oldlayout=NULL);
	virtual ~Layout_NUp();
	const char *GetType();
	int GetCapabilities();
	int AddImage(const char *filename,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO);
	void CopyImage(Layout_ImageInfo *ii);
	bool PlaceImage(const char *filename,int page,int row, int column,bool cropfit,PP_ROTATION rotate);
	void FindFirstFree(int &page,int &row,int &column);
	int FreeSlots();
	void Reflow();
	virtual void LayoutToDB(LayoutDB &db);
	virtual void DBToLayout(LayoutDB &db);
	virtual GtkWidget *CreateWidget();
	virtual void RefreshWidget(GtkWidget *widget);
	virtual ImageSource *GetImageSource(int page,CMColourDevice target=CM_COLOURDEVICE_PRINTER,
		CMTransformFactory *factory=NULL,int res=0,bool completepage=false);
	Layout_NUp_ImageInfo *ImageAt(int page, int row, int column);
	virtual void (*SetUnitFunc())(GtkWidget *wid,enum Units unit);
	virtual Layout_ImageInfo *ImageAtCoord(int x,int y);
	friend class Layout_NUp_ImageInfo;
};


class Layout_NUp_ImageInfo : public Layout_ImageInfo
{
	public:
	Layout_NUp_ImageInfo(Layout_NUp &layout,const char *filename,int row,int column,int page,bool allowcropping=false, PP_ROTATION rotation=PP_ROTATION_AUTO);
	Layout_NUp_ImageInfo(Layout_NUp &layout,Layout_ImageInfo *ii,int row,int column,int page);
	virtual ~Layout_NUp_ImageInfo();
	virtual LayoutRectangle *GetBounds();
	void DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height);
	int row;
	int column;
	private:
	friend class Layout_NUp;
};


#endif
