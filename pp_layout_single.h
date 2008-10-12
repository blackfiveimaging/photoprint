#ifndef __PP_LAYOUT_SINGLE_H__
#define __PP_LAYOUT_SINGLE_H__


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

#define PP_LAYOUT_SINGLE_TYPE			(pp_layout_single_get_type())
#define PP_LAYOUT_SINGLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_LAYOUT_SINGLE_TYPE, pp_Layout_Single))
#define PP_LAYOUT_SINGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_LAYOUT_SINGLE_TYPE, pp_Layout_SingleClass))
#define IS_PP_LAYOUT_SINGLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_LAYOUT_SINGLE_TYPE))
#define IS_PP_LAYOUT_SINGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_LAYOUT_SINGLE_TYPE))

typedef struct _pp_Layout_Single pp_Layout_Single;
typedef struct _pp_Layout_SingleClass pp_Layout_SingleClass;

struct _pp_Layout_Single
{
	GtkHBox hbox;
	GtkWidget *pageview;
	GtkWidget *page;
	GtkWidget *pageextent;
	GtkWidget *hscale;
	GtkWidget *vscale;
	GtkWidget *imagecontrol;
	GtkSizeGroup *sizegroup;
	PhotoPrint_State *state;
};


struct _pp_Layout_SingleClass
{
	GtkHBoxClass parent_class;

	void (*changed)(pp_Layout_Single *book);
	void (*popupmenu)(pp_Layout_Single *book);
	void (*selection_changed)(pp_Layout_Single *book);
};

GType pp_layout_single_get_type (void);
GtkWidget* pp_layout_single_new (PhotoPrint_State *state);
void pp_layout_single_refresh(pp_Layout_Single *ob);
void pp_layout_single_set_unit(GtkWidget *wid,enum Units unit);

G_END_DECLS

#endif /* __PP_LAYOUT_SINGLE_H__ */
