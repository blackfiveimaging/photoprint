#ifndef __STPUI_COMBO_H__
#define __STPUI_COMBO_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkhbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_COMBO_TYPE			(stpui_combo_get_type())
#define STPUI_COMBO(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_COMBO_TYPE, stpui_Combo))
#define STPUI_COMBO_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_COMBO_TYPE, stpui_ComboClass))
#define IS_STPUI_COMBO(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_COMBO_TYPE))
#define IS_STPUI_COMBO_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_COMBO_TYPE))

typedef struct _stpui_Combo stpui_Combo;
typedef struct _stpui_ComboClass stpui_ComboClass;

struct _stpui_Combo
{
	GtkHBox box;
	GtkWidget *combo;
	GList *optionlist;
	stp_vars_t *vars;
	const char *optionname;
	GtkCheckButton *checkbutton;
	GtkWidget *label;
};


struct _stpui_ComboClass
{
	GtkHBoxClass parent_class;

	void (*changed)(stpui_Combo *combo);
};

GType stpui_combo_get_type (void);
GtkWidget* stpui_combo_new (stp_vars_t *vars,const char *optname,GtkWidget *label);
gboolean stpui_combo_refresh(stpui_Combo *c);

G_END_DECLS

#endif /* __STPUI_COMBO_H__ */
