#ifndef LAYOUT_CAROUSEL_H
#define LAYOUT_CAROUSEL_H

#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "support/circlemontage.h"
#include "support/pageextent.h"
#include "imagesource/imagesource.h"
#include "gp_cppsupport/gprinter.h"

#include "layoutdb.h"
#include "layout.h"

class Layout_Carousel_ImageInfo;
class PhotoPrint_State;

class Layout_Carousel : public Layout
{
	public:
	Layout_Carousel(PhotoPrint_State &state,Layout *oldlayout=NULL);
	virtual ~Layout_Carousel();
	const char *GetType();
	int GetCapabilities();
	int AddImage(const char *filename,bool allowcropping=true,PP_ROTATION rotation=PP_ROTATION_AUTO);
	void CopyImage(Layout_ImageInfo *ii);
	void PlaceImage(const char *filename);
	int FreeSlots();
	void Reflow();
	virtual void LayoutToDB(LayoutDB &db);
	virtual void DBToLayout(LayoutDB &db);
	virtual GtkWidget *CreateWidget();
	virtual void RefreshWidget(GtkWidget *widget);
	virtual ImageSource *GetImageSource(int page,CMColourDevice target=CM_COLOURDEVICE_PRINTER,
		CMTransformFactory *factory=NULL,int res=0,bool completepage=false);
	Layout_Carousel_ImageInfo *ImageAt(int page, int segment);
	Layout_ImageInfo *ImageAtCoord(int x,int y);
	virtual void (*SetUnitFunc())(GtkWidget *wid,enum Units unit);
	int CountImages(int page);
	Layout_Carousel_ImageInfo *GetNthImage(int page,int n);
	void FlushPreview();
	void DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height);
	void SetSegments(int segs);
	int GetSegments();
	void SetOverlap(int overlap);
	int GetOverlap();
	void SetAngleOffset(int ao);
	int GetAngleOffset();
	void SetInnerRadius(int ir);
	int GetInnerRadius();
	private:
	int segments;
	int overlap;
	int angleoffset;
	int innerradius;
	void RenderPreview(int width,int height);
	GdkPixbuf *preview;
	int prevwidth,prevheight;
	friend class Layout_Carousel_ImageInfo;
};


class Layout_Carousel_ImageInfo : public Layout_ImageInfo
{
	public:
	Layout_Carousel_ImageInfo(Layout_Carousel &layout,const char *filename,int page,bool allowcropping=false, PP_ROTATION rotation=PP_ROTATION_AUTO);
	Layout_Carousel_ImageInfo(Layout_Carousel &layout,Layout_ImageInfo *ii,int page);
	virtual ~Layout_Carousel_ImageInfo();
	void DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height);
	void FlushThumbnail();
	virtual LayoutRectangle *GetBounds();
	int segment;
	private:
	friend class Layout_Carousel;
};


#endif
