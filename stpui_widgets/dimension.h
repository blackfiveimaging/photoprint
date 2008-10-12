#ifndef __DIMENSION_H__
#define __DIMENSION_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtktooltips.h>

#include <gutenprint/gutenprint.h>

#include "units.h"

#include "../config.h"
#include "../gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

G_BEGIN_DECLS

#define DIMENSION_TYPE			(dimension_get_type())
#define DIMENSION(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), DIMENSION_TYPE, Dimension))
#define DIMENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), DIMENSION_TYPE, DimensionClass))
#define IS_DIMENSION(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), DIMENSION_TYPE))
#define IS_DIMENSION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), DIMENSION_TYPE))

typedef struct _Dimension Dimension;
typedef struct _DimensionClass DimensionClass;

struct _Dimension
{
	GtkHBox	box;
	GtkWidget *spinbutton;
	GtkWidget *label;
	GtkTooltips *tooltips;
	enum Units unit;
	gdouble minpt,maxpt,steppt;
	int value;
};


struct _DimensionClass
{
	GtkHBoxClass parent_class;

	void (*changed)(Dimension *sig);
};

GType dimension_get_type (void);
GtkWidget* dimension_new (gdouble min,gdouble max,enum Units unit);
void dimension_refresh(Dimension *ob);
int dimension_get_pt(Dimension *ob);
void dimension_update_unit_label(Dimension *ob);
void dimension_set_unit(Dimension *ob,enum Units unit);
void dimension_set_pt(Dimension *ob,int pt);
void dimension_set_range_pt(Dimension *ob,int low,int high);
void dimension_set_value(Dimension *ob,gdouble val,enum Units unit);
void dimension_show_unit(Dimension *ob);
void dimension_hide_unit(Dimension *ob);

G_END_DECLS

#endif /* __DIMENSION_H__ */
