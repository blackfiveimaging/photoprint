#ifndef LAYOUT_POSTER_H
#define LAYOUT_POSTER_H

#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "imagesource/imagesource.h"
#include "support/pageextent.h"
#include "gp_cppsupport/gprinter.h"

#include "layoutdb.h"
#include "layout.h"

class Layout_Poster_ImageInfo;
class PhotoPrint_State;

struct PosterFit
{
	int width,height;
	int xpos,ypos;
	int xoffset,yoffset;
	int rotation;
	double scale;
};

class Layout_Poster : public Layout
{
	public:
	Layout_Poster(PhotoPrint_State &state,Layout *oldlayout=NULL);
	virtual ~Layout_Poster();
	const char *GetType();
	int GetCapabilities();
	int AddImage(const char *filename,bool allowcropping=true,PP_ROTATION rotation=PP_ROTATION_AUTO);
	void CopyImage(Layout_ImageInfo *ii);
	void Reflow();
	void SetPageExtent(PageExtent &pe);
	void SetMargins(int left,int right,int top,int bottom);
	void TilesFromSize();
	void SizeFromTiles();
	virtual void LayoutToDB(LayoutDB &db);
	virtual void DBToLayout(LayoutDB &db);
	virtual GtkWidget *CreateWidget();
	virtual void RefreshWidget(GtkWidget *widget);
	virtual void SetCurrentPage(int page);
	ImageSource *GetImageSource(int page,CMColourDevice target=CM_COLOURDEVICE_PRINTER,
		CMTransformFactory *factory=NULL,int res=0,bool completepage=false);
	virtual Layout_ImageInfo *FirstSelected();
	virtual Layout_ImageInfo *NextSelected();
	Layout_Poster_ImageInfo *ImageAt(int page);
	void DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height);
	virtual void (*SetUnitFunc())(GtkWidget *wid,enum Units unit);
	int posters,currentposter;
	int posterwidth,posterheight;
	int paperwidth,paperheight;
	int hoverlap,voverlap;
	int htiles,vtiles;
	friend class Layout_Poster_ImageInfo;
};


class Layout_Poster_ImageInfo : public Layout_ImageInfo
{
	public:
	Layout_Poster_ImageInfo(Layout_Poster &layout,const char *filename,int page,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO);
	Layout_Poster_ImageInfo(Layout_Poster &layout,Layout_ImageInfo *ii,int page);
	virtual ~Layout_Poster_ImageInfo();
	virtual LayoutRectangle *GetBounds();
	void DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height);
//	ImageSource *GetImageSource();
//	int rotation;
	private:
	friend class Layout_Poster;
};


#endif
