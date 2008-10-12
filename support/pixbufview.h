#ifndef __PIXBUFVIEW_H__
#define __PIXBUFVIEW_H__


#include <gdk/gdk.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PIXBUFVIEW(obj)          GTK_CHECK_CAST (obj, pixbufview_get_type (), PixbufView)
#define PIXBUFVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, pixbufview_get_type (), PixbufViewClass)
#define IS_PIXBUFVIEW(obj)       GTK_CHECK_TYPE (obj, pixbufview_get_type ())


typedef struct _PixbufView        PixbufView;
typedef struct _PixbufViewClass   PixbufViewClass;


struct _PixbufView
{
	GtkWidget wid;
	GdkPixbuf *pb;
	GdkPixbuf *pb_scaled;
	bool resized;
	bool scaletofit;
	int xoffset,yoffset;
	bool dragging;
	int prev_x,prev_y;
};


struct _PixbufViewClass
{
	GtkWidgetClass parent_class;
	
	void (*changed)(PixbufView *pv);
	void (*popupmenu)(PixbufView *pv);
};


GtkWidget* pixbufview_new(GdkPixbuf *pb,bool scaletofit=true);
GtkType pixbufview_get_type(void);
void pixbufview_set_pixbuf(PixbufView *pv,GdkPixbuf *pb);
void pixbufview_refresh(PixbufView *pv);

int pixbufview_get_xoffset(PixbufView *pv);
int pixbufview_get_yoffset(PixbufView *pv);
bool pixbufview_get_scale(PixbufView *pv);

void pixbufview_set_offset(PixbufView *pv,int xoff,int yoff);
void pixbufview_set_scale(PixbufView *pv,bool scaletofit);

G_END_DECLS

#endif /* __PIXBUFVIEW_H__ */
