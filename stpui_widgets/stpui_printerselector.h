#ifndef __STPUI_PRINTERSELECTOR_H__
#define __STPUI_PRINTERSELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkhbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_PRINTERSELECTOR_TYPE			(stpui_printerselector_get_type())
#define STPUI_PRINTERSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_PRINTERSELECTOR_TYPE, stpui_PrinterSelector))
#define STPUI_PRINTERSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_PRINTERSELECTOR_TYPE, stpui_PrinterSelectorClass))
#define IS_STPUI_PRINTERSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_PRINTERSELECTOR_TYPE))
#define IS_STPUI_PRINTERSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_PRINTERSELECTOR_TYPE))

typedef struct _stpui_PrinterSelector stpui_PrinterSelector;
typedef struct _stpui_PrinterSelectorClass stpui_PrinterSelectorClass;

struct _stpui_PrinterSelector
{
	GtkHBox box;
	GtkWidget *mancombo;
	GtkWidget *modelcombo;
	GList *manufacturers;
	int manufacturercount;
	GList *models;
	int modelcount;
	char *manufacturer;
	char *model;
};


struct _stpui_PrinterSelectorClass
{
	GtkHBoxClass parent_class;

	void (*changed)(stpui_PrinterSelector *combo);
};

GType stpui_printerselector_get_type (void);
GtkWidget* stpui_printerselector_new ();
const char *stpui_printerselector_get_printer_name(stpui_PrinterSelector *c);
const char *stpui_printerselector_get_driver(stpui_PrinterSelector *c);
void stpui_printerselector_set_printer_name(stpui_PrinterSelector *c,const char *name);
void stpui_printerselector_set_driver(stpui_PrinterSelector *c,const char *driver);
G_END_DECLS

#endif /* __STPUI_PRINTERSELECTOR_H__ */
