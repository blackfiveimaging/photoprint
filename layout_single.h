#ifndef LAYOUT_SINGLE_H
#define LAYOUT_SINGLE_H

#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "imagesource/imagesource.h"
#include "support/pageextent.h"
#include "gp_cppsupport/gprinter.h"

#include "layoutdb.h"
#include "layout.h"

class Layout_Single_ImageInfo;
class LayoutIterator_Single;
class PhotoPrint_State;

class Layout_Single : public Layout
{
	public:
	Layout_Single(PhotoPrint_State &state,Layout *oldlayout=NULL);
	virtual ~Layout_Single();
	const char *GetType();
	int GetCapabilities();
	int AddImage(const char *filename,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO,bool fliphorizontal=false,bool flipvertical=false);
	void CopyImage(Layout_ImageInfo *ii);
	void Reflow();
	void SetPageExtent(PageExtent &pe);
	virtual void LayoutToDB(LayoutDB &db);
	virtual void DBToLayout(LayoutDB &db);
	virtual GtkWidget *CreateWidget();
	virtual void RefreshWidget(GtkWidget *widget);
	virtual void Print(Progress *p);	// Overridden so we can set the top/left position...
	virtual ImageSource *GetImageSource(int page,CMColourDevice target=CM_COLOURDEVICE_PRINTER,
		CMTransformFactory *factory=NULL,int res=0,bool completepage=false);
	Layout_Single_ImageInfo *ImageAt(int page);
	virtual void (*SetUnitFunc())(GtkWidget *wid,enum Units unit);
	friend class Layout_Single_ImageInfo;
	friend class LayoutIterator_Single;
};


class Layout_Single_ImageInfo : public Layout_ImageInfo
{
	public:
	Layout_Single_ImageInfo(Layout_Single &layout,const char *filename,int page,bool allowcropping=false,
		PP_ROTATION rotation=PP_ROTATION_AUTO,bool fliphorizontal=false,bool flipvertical=false);
	Layout_Single_ImageInfo(Layout_Single &layout,Layout_ImageInfo *ii,int page);
	virtual ~Layout_Single_ImageInfo();
	void DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height);
	virtual ImageSource *GetImageSource(CMColourDevice target=CM_COLOURDEVICE_PRINTER,CMTransformFactory *factory=NULL);
	virtual LayoutRectangle *GetBounds();	// The dimensions of the image's "slot".
	virtual RectFit *GetFit(double scale);	// Details of the image's size after fitting to its slot.
	virtual bool GetSelected();
	float hscale;
	float vscale;
	private:
	void Init();
	friend class Layout_Single;
	friend class LayoutIterator_Single;
};


#endif
