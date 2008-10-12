#ifndef __PROFILESELECTOR_H__
#define __PROFILESELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktooltips.h>

#include "profilemanager.h"

G_BEGIN_DECLS

#define PROFILESELECTOR_TYPE			(profileselector_get_type())
#define PROFILESELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PROFILESELECTOR_TYPE, ProfileSelector))
#define PROFILESELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PROFILESELECTOR_TYPE, ProfileSelectorClass))
#define IS_PROFILESELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PROFILESELECTOR_TYPE))
#define IS_PROFILESELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PROFILESELECTOR_TYPE))

typedef struct _ProfileSelector ProfileSelector;
typedef struct _ProfileSelectorClass ProfileSelectorClass;

struct _ProfileSelector
{
	GtkVBox box;
//	GtkWidget *description;
	GtkWidget *optionmenu;
	GtkWidget *menu;
	GList *optionlist;
	GtkListStore *liststore;
	GtkWidget *combobox;
	GtkTooltips *tips;
	ProfileManager *pm;
	IS_TYPE colourspace;
	bool allowdevicelink;
	gchar *filename;
	int currentidx;
};


struct _ProfileSelectorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(ProfileSelector *combo);
};

GType profileselector_get_type (void);

GtkWidget* profileselector_new (ProfileManager *pm,IS_TYPE colourspace=IS_TYPE_NULL,bool allowdevicelink=false);
// Pass IS_TYPE_NULL to match any colourspace

const char *profileselector_get_filename(ProfileSelector *c);
void profileselector_set_filename(ProfileSelector *c, const char *filename);
void profileselector_set_type(ProfileSelector *c,IS_TYPE colourspace=IS_TYPE_NULL);
gboolean profileselector_refresh(ProfileSelector *c);

G_END_DECLS

#endif /* __PROFILESELECTOR_H__ */
