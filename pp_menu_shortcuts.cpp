#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include "pp_menu_shortcuts.h"
#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "support/progressbar.h"
#include "support/pathsupport.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


static void file_profiling_mode(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;	
	char *fn=substitute_homedir("$HOME" SEARCHPATH_SEPARATOR_S ".photoprint" SEARCHPATH_SEPARATOR_S "profiling.preset");
	mw->state->ParseSupplementaryConfig(fn);
	free(fn);
	pp_mainwindow_rebuild(mw);
}


static GtkActionEntry shortcutsmenu_entries[] = {
  { "ShortcutsMenu", NULL, N_("_Shortcuts") },

  { "ProfilingDefaults", NULL, N_("Pro_filing Defaults"), NULL, N_("Profiling defaults"), G_CALLBACK(file_profiling_mode) },
};

static const char *shortcutsmenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ShortcutsMenu'>"
"      <menuitem action='ProfilingDefaults'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildShortcutsMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("ShortcutsMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, shortcutsmenu_entries, G_N_ELEMENTS (shortcutsmenu_entries), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, shortcutsmenu_ui_description, -1, &error))
		throw error->message;
}

