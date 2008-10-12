#ifndef __PP_LAYOUT_SINGLE_PAGEVIEW_H__
#define __PP_LAYOUT_SINGLE_PAGEVIEW_H__


#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmenuitem.h>

#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"

#include "layout_single.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define PP_LAYOUT_SINGLE_PAGEVIEW(obj)          GTK_CHECK_CAST (obj, pp_layout_single_pageview_get_type (), pp_Layout_Single_PageView)
#define PP_LAYOUT_SINGLE_PAGEVIEW_CLASS(klass)  GTK_CHECK_CLASS_CAST (klass, pp_layout_single_pageview_get_type (), pp_Layout_Single_PageViewClass)
#define PP_IS_PAGEVIEW(obj)       GTK_CHECK_TYPE (obj, pp_layout_single_pageview_get_type ())


typedef struct _pp_Layout_Single_PageView        pp_Layout_Single_PageView;
typedef struct _pp_Layout_Single_PageViewClass   pp_Layout_Single_PageViewClass;


struct _pp_Layout_Single_PageView
{
	GtkWidget widget;

	/* Button currently pressed or 0 if none */
	guint8 button;

	/* ID of update timer, or 0 if none */
	guint32 timer;

	Layout_Single *layout;
	Layout_Single_ImageInfo *selected;
//	GList *imagelist;

	int left, top;
	int width, height;

	double scale;
};

struct _pp_Layout_Single_PageViewClass
{
	GtkWidgetClass parent_class;
	
	void (*changed)(pp_Layout_Single_PageView *pv);
	void (*reflow)(pp_Layout_Single_PageView *pv);
	void (*popupmenu)(pp_Layout_Single_PageView *pv);
	void (*selection_changed)(pp_Layout_Single_PageView *pv);
};


GtkWidget* pp_layout_single_pageview_new(Layout_Single *layout);
GtkType pp_layout_single_pageview_get_type(void);
void pp_layout_single_pageview_set_page(pp_Layout_Single_PageView *pv,int page);
void pp_layout_single_pageview_refresh(pp_Layout_Single_PageView *pv);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __PP_LAYOUT_SINGLE_PAGEVIEW_H__ */
