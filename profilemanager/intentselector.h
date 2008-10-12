#ifndef __INTENTSELECTOR_H__
#define __INTENTSELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtktooltips.h>

#include "profilemanager.h"

G_BEGIN_DECLS

#define INTENTSELECTOR_TYPE			(intentselector_get_type())
#define INTENTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), INTENTSELECTOR_TYPE, IntentSelector))
#define INTENTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), INTENTSELECTOR_TYPE, IntentSelectorClass))
#define IS_INTENTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), INTENTSELECTOR_TYPE))
#define IS_INTENTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), INTENTSELECTOR_TYPE))

typedef struct _IntentSelector IntentSelector;
typedef struct _IntentSelectorClass IntentSelectorClass;

struct _IntentSelector
{
	GtkHBox box;
	GtkWidget *optionmenu;
	GtkWidget *menu;
	ProfileManager *pm;
	GtkTooltips *tips;
};


struct _IntentSelectorClass
{
	GtkHBoxClass parent_class;

	void (*changed)(IntentSelector *combo);
};

GType intentselector_get_type (void);
GtkWidget* intentselector_new (ProfileManager *pm);
LCMSWrapper_Intent intentselector_getintent(IntentSelector *c);
void intentselector_setintent(IntentSelector *c,LCMSWrapper_Intent intent);
G_END_DECLS

#endif /* __INTENTSELECTOR_H__ */
