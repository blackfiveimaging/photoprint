#ifndef __PP_LAYOUT_CAROUSEL_H__
#define __PP_LAYOUT_CAROUSEL_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>

#include <gutenprint/gutenprint.h>

#include "stpui_widgets/units.h"
#include "photoprint_state.h"
#include "pp_sigcontrol.h"

G_BEGIN_DECLS

#define PP_LAYOUT_CAROUSEL_TYPE			(pp_layout_carousel_get_type())
#define PP_LAYOUT_CAROUSEL(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_LAYOUT_CAROUSEL_TYPE, pp_Layout_Carousel))
#define PP_LAYOUT_CAROUSEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_LAYOUT_CAROUSEL_TYPE, pp_Layout_CarouselClass))
#define IS_PP_LAYOUT_CAROUSEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_LAYOUT_CAROUSEL_TYPE))
#define IS_PP_LAYOUT_CAROUSEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_LAYOUT_CAROUSEL_TYPE))

typedef struct _pp_Layout_Carousel pp_Layout_Carousel;
typedef struct _pp_Layout_CarouselClass pp_Layout_CarouselClass;

struct _pp_Layout_Carousel
{
	GtkHBox hbox;
	GtkWidget *pageview;
	GtkWidget *page;
	GtkWidget *pageextent;
	GtkWidget *segments;
	GtkWidget *overlap;
	GtkWidget *angleoffset;
	GtkWidget *innerradius;
	GtkWidget *imagecontrol;
	PhotoPrint_State *state;
};


struct _pp_Layout_CarouselClass
{
	GtkHBoxClass parent_class;

	void (*changed)(pp_Layout_Carousel *book);
	void (*popupmenu)(pp_Layout_Carousel *book);
	void (*selection_changed)(pp_Layout_Carousel *book);
};

GType pp_layout_carousel_get_type (void);
GtkWidget* pp_layout_carousel_new (PhotoPrint_State *state);
void pp_layout_carousel_refresh(pp_Layout_Carousel *ob);
void pp_layout_carousel_set_unit(GtkWidget *wid,enum Units unit);

G_END_DECLS

#endif /* __PP_LAYOUT_CAROUSEL_H__ */
