#ifndef LAYOUT_IMAGEINFO_H
#define LAYOUT_IMAGEINFO_H

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
#include "effects/ppeffect.h"

#include "layoutdb.h"

class Layout;
class PhotoPrint_State;
class Progress;

class hr_payload;
class Layout_ImageInfo : public PPEffectHeader
{
	public:
	Layout_ImageInfo(Layout &layout,const char *filename,int page,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO);
	Layout_ImageInfo(Layout &layout,Layout_ImageInfo *oldii,int page,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO);

	// Housekeeping

	virtual ~Layout_ImageInfo();
	virtual bool GetSelected();
	virtual void SetSelected(bool sel);
	virtual void ToggleSelected();
	virtual void ObtainMutex();		// Overriding the inherited ObtainMutex exclusive lock
									// method allows for any running preview-generating
									// threads to be cancelled before locking for writing.

	// Image details

	virtual const char *GetFilename();
	virtual int GetWidth();
	virtual int GetHeight();
	virtual int GetXRes();
	virtual int GetYRes();
	virtual LayoutRectangle *GetBounds();	// The dimensions of the image's "slot".
	virtual RectFit *GetFit(double scale);	// Details of the image's size after fitting to its slot.
	virtual void SetMask(const char *filename);
	virtual ImageSource *ApplyMask(ImageSource *is);
	virtual void AssignProfile(const char *filename);
	virtual const char *GetAssignedProfile();
	virtual void SetRenderingIntent(LCMSWrapper_Intent intent);
	virtual LCMSWrapper_Intent GetRenderingIntent();
	virtual ImageSource *GetImageSource(CMColourDevice target=CM_COLOURDEVICE_PRINTER,CMTransformFactory *factory=NULL);

	// Thumbnail/preview related

	virtual GdkPixbuf *GetThumbnail();
	virtual void DrawThumbnail(GtkWidget *widget,int xpos,int ypos,int width,int height);
	virtual void FlushThumbnail();
	virtual void FlushHRPreview();
	virtual void CancelRenderThread();
	virtual void SetHRPreview(GdkPixbuf *preview); // Called by idle handler once render thread has completed.

	int page;
	bool allowcropping;
	LayoutRectangle_Alignment crop_hpan;
	LayoutRectangle_Alignment crop_vpan;
	enum PP_ROTATION rotation;
	Layout &layout;
	protected:
	char *filename;
	char *maskfilename;
	int width,height;
	double xres,yres;
	GdkPixbuf *thumbnail;
	GdkPixbuf *mask;
	GdkPixbuf *hrpreview;
	bool selected;
	char *customprofile;
	LCMSWrapper_Intent customintent;
	Thread *hrrenderthread;
	friend class Layout;
	friend class hr_payload;
};


#endif