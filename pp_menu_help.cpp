#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "support/progressbar.h"

#include "pp_menu_help.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


static void help_about(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	About_Dialog(GTK_WINDOW(mw));
}


static GtkActionEntry helpmenu_entries[] = {
  { "HelpMenu", NULL, N_("_Help") },

  { "About", NULL, N_("About..."), NULL, N_("Some information about PhotoPrint"), G_CALLBACK(help_about) },
};

static const char *helpmenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildHelpMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("HelpMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, helpmenu_entries, G_N_ELEMENTS (helpmenu_entries), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, helpmenu_ui_description, -1, &error))
		throw error->message;
}
