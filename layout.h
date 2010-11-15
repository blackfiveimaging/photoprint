#ifndef LAYOUT_H
#define LAYOUT_H

#include <list>
#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "profilemanager/profilemanager.h"
#include "imagesource/imagesource.h"
#include "stpui_widgets/units.h"
#include "support/pageextent.h"
#include "support/layoutrectangle.h"
#include "support/thread.h"
#include "support/jobqueue.h"
#include "effects/ppeffect.h"

#include "layoutdb.h"
#include "layout_imageinfo.h"
class Layout_ImageInfo;
class PhotoPrint_State;
class Progress;

// Layout capabilities - needed by the UI:
#define PPLAYOUT_CROP 1
#define PPLAYOUT_ROTATE 2
#define PPLAYOUT_MASK 4
#define PPLAYOUT_PROFILE 8
#define PPLAYOUT_BACKGROUND 16
#define PPLAYOUT_EFFECTS 32
#define PPLAYOUT_DUPLICATE 64


class LayoutIterator
{
	public:
	LayoutIterator(Layout &header);
	~LayoutIterator();
	Layout_ImageInfo *FirstImage();
	Layout_ImageInfo *NextImage();
	Layout_ImageInfo *FirstSelected();
	Layout_ImageInfo *NextSelected();
	protected:
	Layout &header;
	std::list<Layout_ImageInfo *>::iterator iterator;
};


class Layout : public virtual PageExtent
{
	public:

	// Housekeeping

	Layout(PhotoPrint_State &state,Layout *oldlayout=NULL);
	virtual ~Layout();
	virtual void Delete(Layout_ImageInfo *ii);
	virtual void Remove(Layout_ImageInfo *ii);
	virtual const char *GetType()=0;
	virtual int GetCapabilities();
	virtual int AddImage(const char *filename,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO);
	virtual void CopyImage(Layout_ImageInfo *ii);
	virtual void TransferImages(Layout *oldlayout,Progress *p=NULL);
	virtual void Clear();
	virtual int GetPages();
	virtual int FreeSlots();	// Count the number of free slots on the current page
	virtual void Reflow();

	virtual ImageSource *GetImageSource(int page,CMColourDevice target=CM_COLOURDEVICE_PRINTER,
		CMTransformFactory *factory=NULL,int res=0,bool completepage=false);
	virtual IS_TYPE GetColourSpace(CMColourDevice target);	// Do we still need this?
	virtual void UpdatePageSize();
	virtual void LayoutToDB(LayoutDB &db);
	virtual void DBToLayout(LayoutDB &db);

	// Image list / selections

	virtual int CountSelected();
	virtual void SelectAll();
	virtual void SelectNone();
	virtual Layout_ImageInfo *ImageAtCoord(int x,int y);
	virtual int GetCurrentPage();
	virtual void SetCurrentPage(int page);

	// UI-related

	virtual GtkWidget *CreateWidget();
	virtual void RefreshWidget(GtkWidget *widget);
	virtual void Print(Progress *p);
	virtual void DrawGridLines(GtkWidget *widget);
	virtual void DrawPreviewBorder(GtkWidget *widget);
	virtual void DrawPreviewBG(GtkWidget *widget,int xpos,int ypos,int width,int height);
	virtual void DrawPreview(GtkWidget *widget,int xpos,int ypos,int width,int height);
	virtual void SetBackground(const char *filename);
	virtual void FlushThumbnails();
	virtual void FlushHRPreviews();
	virtual void CancelRenderThreads();
	virtual void (*SetUnitFunc())(GtkWidget *wid,enum Units unit);

	PhotoPrint_State &state;

	protected:
	void MakeGC(GtkWidget *widget);
	// Xoffset and yoffset are the top left corner of the print.
	// Some layouts will want to set this to the top/left margin
	// The best way to do so is to override the print method.
	int xoffset,yoffset;
	int pages;
	int currentpage;
	// Background image
	char *backgroundfilename;
	GdkPixbuf *background;
	GdkPixbuf *backgroundtransformed;

	// Housekeeping
	list<Layout_ImageInfo *> imagelist;

	// For thumbnails and preview...
	CMTransformFactory *factory;
	GdkGC *gc;
	GdkColor bgcol;
	GdkColor bgcol2;

	// JobDispatcher - for tracking the high-res preview rendering.
	JobDispatcher jobdispatcher;

	friend class Layout_ImageInfo;
	friend class hr_payload;
	friend class HRRenderJob;
	friend class LayoutIterator;
};


#endif
