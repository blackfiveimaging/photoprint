#ifndef ROTATEPIXBUF_H
#define ROTATEPIXBUF_H

#include <gdk/gdkpixbuf.h>
#ifndef GDK_PIXBUF_TRANSFORM_H

#define gdk_pixbuf_rotate_simple pv_gdk_pixbuf_rotate_simple

typedef enum {
	GDK_PIXBUF_ROTATE_NONE             =   0,
	GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE =  90,
	GDK_PIXBUF_ROTATE_UPSIDEDOWN       = 180,
	GDK_PIXBUF_ROTATE_CLOCKWISE        = 270
} GdkPixbufRotation;

#endif

GdkPixbuf *
pv_gdk_pixbuf_rotate_simple (const GdkPixbuf   *src,
			  GdkPixbufRotation angle);

#endif
