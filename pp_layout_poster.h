#ifndef __PP_LAYOUT_POSTER_H__
#define __PP_LAYOUT_POSTER_H__


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

#define PP_LAYOUT_POSTER_TYPE			(pp_layout_poster_get_type())
#define PP_LAYOUT_POSTER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_LAYOUT_POSTER_TYPE, pp_Layout_Poster))
#define PP_LAYOUT_POSTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_LAYOUT_POSTER_TYPE, pp_Layout_PosterClass))
#define IS_PP_LAYOUT_POSTER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_LAYOUT_POSTER_TYPE))
#define IS_PP_LAYOUT_POSTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_LAYOUT_POSTER_TYPE))

typedef struct _pp_Layout_Poster pp_Layout_Poster;
typedef struct _pp_Layout_PosterClass pp_Layout_PosterClass;

struct _pp_Layout_Poster
{
	GtkHBox hbox;
	GtkWidget *pageview;
	GtkWidget *page;
	GtkWidget *pageextent;
	GtkWidget *posterwidth;
	GtkWidget *posterheight;
	GtkWidget *hoverlap;
	GtkWidget *voverlap;
	GtkWidget *htiles;
	GtkWidget *vtiles;
	GtkWidget *imagecontrol;

	PhotoPrint_State *state;
};


struct _pp_Layout_PosterClass
{
	GtkHBoxClass parent_class;

	void (*changed)(pp_Layout_Poster *book);
};

GType pp_layout_poster_get_type (void);
GtkWidget* pp_layout_poster_new (PhotoPrint_State *state);
void pp_layout_poster_refresh(pp_Layout_Poster *ob);
void pp_layout_poster_set_unit(GtkWidget *wid,enum Units unit);

G_END_DECLS

#endif /* __PP_LAYOUT_POSTER_H__ */
