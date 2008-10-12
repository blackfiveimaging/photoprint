#ifndef __PP_LAYOUT_NUP_H__
#define __PP_LAYOUT_NUP_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>

#include <gutenprint/gutenprint.h>

#include "stpui_widgets/units.h"
#include "pp_layout_nup_pageview.h"

G_BEGIN_DECLS

#define PP_LAYOUT_NUP_TYPE			(pp_layout_nup_get_type())
#define PP_LAYOUT_NUP(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_LAYOUT_NUP_TYPE, pp_Layout_NUp))
#define PP_LAYOUT_NUP_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_LAYOUT_NUP_TYPE, pp_Layout_NUpClass))
#define IS_PP_LAYOUT_NUP(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_LAYOUT_NUP_TYPE))
#define IS_PP_LAYOUT_NUP_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_LAYOUT_NUP_TYPE))

typedef struct _pp_Layout_NUp pp_Layout_NUp;
typedef struct _pp_Layout_NUpClass pp_Layout_NUpClass;

class PhotoPrint_State;

struct _pp_Layout_NUp
{
	GtkHBox hbox;
	GtkWidget *pageview;
	GtkWidget *sigcontrol;
	GtkWidget *pageextent;
	GtkWidget *imagecontrol;
	GtkWidget *page;
	PhotoPrint_State *state;
};


struct _pp_Layout_NUpClass
{
	GtkHBoxClass parent_class;

	void (*changed)(pp_Layout_NUp *book);
	void (*popupmenu)(pp_Layout_NUp *book);
};

GType pp_layout_nup_get_type (void);
GtkWidget* pp_layout_nup_new (PhotoPrint_State *state);
void pp_layout_nup_refresh(pp_Layout_NUp *ob);
void pp_layout_nup_set_unit(GtkWidget *wid,enum Units unit);

G_END_DECLS

#endif /* __PP_LAYOUT_NUP_H__ */
