#ifndef __PP_IMAGECONTROL_H__
#define __PP_IMAGECONTROL_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>

#include <gutenprint/gutenprint.h>

#include "stpui_widgets/units.h"

G_BEGIN_DECLS

#define PP_IMAGECONTROL_TYPE			(pp_imagecontrol_get_type())
#define PP_IMAGECONTROL(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_IMAGECONTROL_TYPE, pp_ImageControl))
#define PP_IMAGECONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_IMAGECONTROL_TYPE, pp_ImageControlClass))
#define IS_PP_IMAGECONTROL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_IMAGECONTROL_TYPE))
#define IS_PP_IMAGECONTROL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_IMAGECONTROL_TYPE))

typedef struct _pp_ImageControl pp_ImageControl;
typedef struct _pp_ImageControlClass pp_ImageControlClass;

class Layout;

struct _pp_ImageControl
{
	GtkVBox	box;
	GtkWidget *imageinfo;
	GtkWidget *effectselector;
	GtkWidget *histogram;
	GtkWidget *expander1;
	GtkWidget *expander2;
	Layout *layout;
};


struct _pp_ImageControlClass
{
	GtkVBoxClass parent_class;

	void (*changed)(pp_ImageControl *sig);
};

GType pp_imagecontrol_get_type (void);
GtkWidget* pp_imagecontrol_new (Layout *layout);
void pp_imagecontrol_refresh(pp_ImageControl *ob);
void pp_imagecontrol_set_image(pp_ImageControl *ob);

G_END_DECLS

#endif /* __PP_IMAGECONTROL_H__ */
