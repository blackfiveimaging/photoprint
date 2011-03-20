#ifndef LAYOUT_IMAGEINFO_H
#define LAYOUT_IMAGEINFO_H

#include <stdio.h>
#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gdk/gdkpixbuf.h>

#include "profilemanager.h"
#include "imagesource.h"
#include "stpui_widgets/units.h"
#include "pageextent.h"
#include "layoutrectangle.h"
#include "thread.h"
#include "threadevent.h"
#include "jobqueue.h"
#include "effects/ppeffect.h"
#include "cmtransformworker.h"
#include "refcountui.h"

#include "histogram.h"
#include "layoutdb.h"

class Layout;
class PhotoPrint_State;
class Progress;


class ImageInfo_Worker : public CMTransformWorker
{
	public:
	ImageInfo_Worker(JobQueue &queue,ProfileManager &pm) : CMTransformWorker(queue,pm)
	{
	}
	virtual ~ImageInfo_Worker()
	{
	}
};


class HRRenderJob;
class hr_payload;
class Layout_ImageInfo : public PPEffectHeader, public RefCountUI
{
	public:
	Layout_ImageInfo(Layout &layout,const char *filename,int page,bool allowcropping=false,PP_ROTATION rotation=PP_ROTATION_AUTO,bool fliphorizontal=false,bool flipvertical=false);
	Layout_ImageInfo(Layout &layout,Layout_ImageInfo *oldii,int page);

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

	virtual void FlushThumbnail();	// Top-level flush routine - flushes low and high-res previews, and cancels render thread
	virtual void FlushHRPreview();	// Flushes just the high-res preview, cancels thread
	virtual void CancelRenderThread();	// Cancels rendering thread.

										// WARNING - for efficiency, these routines may return before a rendering thread
										// has actually finished running.  Thus, you should call ObtainMutex() on this
										// before freeing anything on which the rendering thread may depend - such as the
										// transform factory.

	virtual void SetHRPreview(GdkPixbuf *preview); // Called by idle handler once render thread has completed.
	virtual PPHistogram &GetHistogram();

	int page;
	bool allowcropping;
	bool fliphorizontal;
	bool flipvertical;
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
//	hr_payload *hrrenderthread;
	ThreadEventHandler threadevents;
	PPHistogram histogram;
	Job *hrrenderjob;
	friend class Layout;
	friend class hr_payload;
	friend class HRRenderJob;
};


#endif
