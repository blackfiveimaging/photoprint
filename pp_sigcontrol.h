#ifndef __PP_SIGCONTROL_H__
#define __PP_SIGCONTROL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>

#include <gutenprint/gutenprint.h>

#include "stpui_widgets/units.h"

#include "layout_nup.h"

G_BEGIN_DECLS

#define PP_SIGCONTROL_TYPE			(pp_sigcontrol_get_type())
#define PP_SIGCONTROL(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_SIGCONTROL_TYPE, pp_SigControl))
#define PP_SIGCONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_SIGCONTROL_TYPE, pp_SigControlClass))
#define IS_PP_SIGCONTROL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_SIGCONTROL_TYPE))
#define IS_PP_SIGCONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_SIGCONTROL_TYPE))

typedef struct _pp_SigControl pp_SigControl;
typedef struct _pp_SigControlClass pp_SigControlClass;


struct _pp_SigControl
{
	GtkExpander box;
	GtkWidget *vbox;
	GtkWidget *combo;
	GtkWidget *table;
	GtkWidget *rows;
	GtkWidget *cols;
	GtkWidget *hgutter;
	GtkWidget *vgutter;
	GtkWidget *width;
	GtkWidget *height;
	enum Units unit;
	Layout_NUp *sig;
};


struct _pp_SigControlClass
{
	GtkExpanderClass parent_class;

	void (*changed)(pp_SigControl *sig);
	void (*reflow)(pp_SigControl *sig);
};

GType pp_sigcontrol_get_type (void);
GtkWidget* pp_sigcontrol_new (Layout_NUp *sig,enum Units unit=UNIT_POINTS);
void pp_sigcontrol_refresh(pp_SigControl *ob);
void pp_sigcontrol_set_unit(pp_SigControl *ob,enum Units unit);
int pp_sigcontrol_get_expander_state(pp_SigControl *ob);
void pp_sigcontrol_set_expander_state(pp_SigControl *ob,int state);

G_END_DECLS

#endif /* __PP_SIGCONTROL_H__ */
