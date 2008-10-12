#ifndef __PP_UNITS_H__
#define __PP_UNITS_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkspinbutton.h>

#include "lcmswrapper.h"
#include "stpui_widgets/units.h"

G_BEGIN_DECLS

#define PP_UNITS_TYPE			(pp_units_get_type())
#define PP_UNITS(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_UNITS_TYPE, pp_Units))
#define PP_UNITS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_UNITS_TYPE, pp_UnitsClass))
#define IS_PP_UNITS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_UNITS_TYPE))
#define IS_PP_UNITS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_UNITS_TYPE))

typedef struct _pp_Units pp_Units;
typedef struct _pp_UnitsClass pp_UnitsClass;

struct _pp_Units
{
	GtkVBox	vbox;
	GtkWidget *unitselector;
};


struct _pp_UnitsClass
{
	GtkVBoxClass parent_class;

	void (*changed)(pp_Units *units);
};

GType pp_units_get_type (void);
GtkWidget* pp_units_new ();
void pp_units_refresh(pp_Units *ob);
void pp_units_set_unit(pp_Units *ob,enum Units unit);
enum Units pp_units_get_unit(pp_Units *ob);
enum Units pp_units_run_dialog(enum Units unit,GtkWindow *parent);

G_END_DECLS

#endif /* __PP_UNITS_H__ */
