#ifndef __PP_MENU_H__
#define __PP_MENU_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkitemfactory.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkmenuitem.h>

#include <gutenprint/gutenprint.h>

#include "layout.h"

G_BEGIN_DECLS

#define PP_MENU_TYPE			(pp_menu_get_type())
#define PP_MENU(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_MENU_TYPE, pp_Menu))
#define PP_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_MENU_TYPE, pp_MenuClass))
#define IS_PP_MENU(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_MENU_TYPE))
#define IS_PP_MENU_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_MENU_TYPE))

typedef struct _pp_Menu pp_Menu;
typedef struct _pp_MenuClass pp_MenuClass;


struct _pp_Menu
{
	GtkHBox	box;
	GtkAccelGroup *accels;
	GtkItemFactory *itemfactory;
	GtkWidget *menubar;
	GtkAccelGroup *popupaccels;
	GtkItemFactory *popupitemfactory;
	GtkWidget *popupmenubar;
	bool allowcropping;
	enum PP_ROTATION rotation;
	bool active;  // FIXME - would need to be a mutex if threaded...
};


struct _pp_MenuClass
{
	GtkHBoxClass parent_class;

	void (*changed)(pp_Menu *book);
};

GType pp_menu_get_type (void);
GtkWidget* pp_menu_new ();
void pp_menu_refresh(pp_Menu *ob);
void pp_menu_set_layout(pp_Menu *ob,const char *type);
GtkAccelGroup *pp_menu_get_accels(pp_Menu *menu);
void pp_menu_set_layout_capabilities(pp_Menu *menu,int features);
void pp_menu_set_menu_state(pp_Menu *menu,bool allowcropping,PP_ROTATION rotation,bool remove);

G_END_DECLS

#endif /* __PP_MENU_H__ */
