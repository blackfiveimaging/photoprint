#ifndef __STPUI_SLIDER_H__
#define __STPUI_SLIDER_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhscale.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_SLIDER_TYPE			(stpui_slider_get_type())
#define STPUI_SLIDER(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_SLIDER_TYPE, stpui_Slider))
#define STPUI_SLIDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_SLIDER_TYPE, stpui_SliderClass))
#define IS_STPUI_SLIDER(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_SLIDER_TYPE))
#define IS_STPUI_SLIDER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_SLIDER_TYPE))

typedef struct _stpui_Slider stpui_Slider;
typedef struct _stpui_SliderClass stpui_SliderClass;

struct _stpui_Slider
{
	GtkHBox box;
	GtkWidget *scale;
	GtkWidget *spin;
	GtkCheckButton *checkbutton;
	GtkWidget *label;
	stp_vars_t *vars;
	const char *optionname;
	int type;
};


struct _stpui_SliderClass
{
	GtkHBoxClass parent_class;

	void (*changed)(stpui_Slider *combo);
};

GType stpui_slider_get_type (void);
GtkWidget* stpui_slider_new (stp_vars_t *vars,const char *optname,GtkWidget *checkbutton);
gboolean stpui_slider_refresh(stpui_Slider *c);

G_END_DECLS

#endif /* __STPUI_SLIDER_H__ */
