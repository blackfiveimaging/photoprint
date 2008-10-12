#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include "pp_menu_edit.h"
#include "pp_mainwindow.h"
#include "dialogs.h"
#include "support/generaldialogs.h"
#include "support/progressbar.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


static void edit_selectall(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	mw->state->layout->SelectAll();
	pp_mainwindow_refresh(mw);
}


static void edit_selectnone(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	mw->state->layout->SelectNone();
	pp_mainwindow_refresh(mw);
}


static GtkActionEntry editmenu_entries[] = {
  { "EditMenu", NULL, N_("_Edit") },
  
  { "SelectAll", NULL, N_("Select _All"), "<control>A", N_("Select all images"), G_CALLBACK(edit_selectall) },
  { "SelectNone", NULL, N_("Select _None"), NULL, N_("Deselect all images"), G_CALLBACK(edit_selectnone) },
  { "Cut", GTK_STOCK_CUT, N_("_Cut"), "<control>X", N_("Cut the current image to the clipboard"), NULL },
  { "Copy", GTK_STOCK_COPY, N_("C_opy"), "<control>C", N_("Copy the current image to the clipboard"), NULL },
  { "Paste", GTK_STOCK_PASTE, N_("_Paste"), "<control>V", N_("Paste the contents of the clipboard as a new image"), NULL },
};

static const char *editmenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='EditMenu'>"
"      <menuitem action='SelectAll'/>"
"      <menuitem action='SelectNone'/>"
"      <separator/>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Paste'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildEditMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("EditMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, editmenu_entries, G_N_ELEMENTS (editmenu_entries), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, editmenu_ui_description, -1, &error))
		throw error->message;
}
