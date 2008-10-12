#ifndef __STPUI_OPTIONPAGE_H__
#define __STPUI_OPTIONPAGE_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>

#include <gutenprint/gutenprint.h>

#include "stpui_combo.h"
#include "stpui_slider.h"
#include "stpui_toggle.h"
#include "stpui_file.h"

G_BEGIN_DECLS

#define STPUI_OPTIONPAGE_TYPE			(stpui_optionpage_get_type())
#define STPUI_OPTIONPAGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_OPTIONPAGE_TYPE, stpui_OptionPage))
#define STPUI_OPTIONPAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_OPTIONPAGE_TYPE, stpui_OptionPageClass))
#define IS_STPUI_OPTIONPAGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_OPTIONPAGE_TYPE))
#define IS_STPUI_OPTIONPAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_OPTIONPAGE_TYPE))

typedef struct _stpui_OptionPage stpui_OptionPage;
typedef struct _stpui_OptionPageClass stpui_OptionPageClass;

struct _stpui_OptionPage
{
	GtkVBox vbox;
	GtkWidget *table;
	GList *widgetlist;
	stp_vars_t *vars;
	int optclass;
	int optlevelmin;
	int optlevelmax;
};


struct _stpui_OptionPageClass
{
	GtkHBoxClass parent_class;

	void (*changed)(stpui_OptionPage *op);
};

GType stpui_optionpage_get_type (void);
GtkWidget* stpui_optionpage_new (stp_vars_t *vars,int optclass,int optlevelmin,int optlevelmax);
gboolean stpui_optionpage_refresh(stpui_OptionPage *op);

G_END_DECLS

#endif /* __STPUI_OPTIONPAGE_H__ */
