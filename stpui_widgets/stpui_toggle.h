#ifndef __STPUI_TOGGLE_H__
#define __STPUI_TOGGLE_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkhbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_TOGGLE_TYPE			(stpui_toggle_get_type())
#define STPUI_TOGGLE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_TOGGLE_TYPE, stpui_Toggle))
#define STPUI_TOGGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_TOGGLE_TYPE, stpui_ToggleClass))
#define IS_STPUI_TOGGLE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_TOGGLE_TYPE))
#define IS_STPUI_TOGGLE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_TOGGLE_TYPE))

typedef struct _stpui_Toggle stpui_Toggle;
typedef struct _stpui_ToggleClass stpui_ToggleClass;

struct _stpui_Toggle
{
	GtkHBox box;
	GtkWidget *toggle;
	stp_vars_t *vars;
	const char *optionname;
};


struct _stpui_ToggleClass
{
	GtkHBoxClass parent_class;

	void (*toggled)(stpui_Toggle *t);
};

GType stpui_toggle_get_type (void);
GtkWidget* stpui_toggle_new (stp_vars_t *vars,const char *optname,const char *displayname);
gboolean stpui_toggle_refresh(stpui_Toggle *t);

G_END_DECLS

#endif /* __STPUI_TOGGLE_H__ */
