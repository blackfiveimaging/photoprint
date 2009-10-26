#ifndef __PP_SCALING_H__
#define __PP_SCALING_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkspinbutton.h>

#include "miscwidgets/simplecombo.h"
#include "imagesource/imagesource_util.h"


G_BEGIN_DECLS

#define PP_SCALING_TYPE			(pp_scaling_get_type())
#define PP_SCALING(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_SCALING_TYPE, pp_Scaling))
#define PP_SCALING_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_SCALING_TYPE, pp_ScalingClass))
#define IS_PP_SCALING(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_SCALING_TYPE))
#define IS_PP_SCALING_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_SCALING_TYPE))

typedef struct _pp_Scaling pp_Scaling;
typedef struct _pp_ScalingClass pp_ScalingClass;

struct _pp_Scaling
{
	GtkVBox	vbox;
	GtkWidget *scaleselector;
	GtkWidget *description;
};


struct _pp_ScalingClass
{
	GtkVBoxClass parent_class;

	void (*changed)(pp_Scaling *units);
};

GType pp_scaling_get_type (void);
GtkWidget* pp_scaling_new (IS_ScalingQuality scale);
void pp_scaling_refresh(pp_Scaling *ob);
void pp_scaling_set_scale(pp_Scaling *ob,IS_ScalingQuality scale);
IS_ScalingQuality pp_scaling_get_scale(pp_Scaling *ob);
IS_ScalingQuality pp_scaling_run_dialog(GtkWindow *parent,IS_ScalingQuality scale);

G_END_DECLS

#endif /* __PP_SCALING_H__ */
