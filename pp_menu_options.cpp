#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkradioaction.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>

#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "support/progressbar.h"

#include "profilemanager/profilemanager.h"

#include "pp_menu_options.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


static void options_paths(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;	
	Paths_Dialog(GTK_WINDOW(mw),*mw->state);
	pp_mainwindow_refresh(mw);
}


static void options_colourmanagement(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;	
	ColourManagement_Dialog(GTK_WINDOW(mw),*mw->state);
	mw->state->layout->FlushThumbnails();
	pp_mainwindow_refresh(mw);
	OptionsMenu_SetProofMode(mw->uim,CMProofMode(mw->state->profilemanager.FindInt("ProofMode")));
}


static void options_units(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	Units_Dialog(GTK_WINDOW(mw),*mw->state);
	(mw->state->layout->SetUnitFunc())(mw->layout,mw->state->GetUnits());
}


static void options_scaling(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	Scaling_Dialog(GTK_WINDOW(mw),*mw->state);
}


static void options_highres(GtkToggleAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	bool checked=gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(act));
	mw->state->SetInt("HighresPreviews",checked);
	if(checked)
		pp_mainwindow_refresh(mw);
}


static gboolean radioidlefunc(gpointer userdata)
{
	pp_MainWindow *mw=(pp_MainWindow *)userdata;
	cerr << "In Idle function - reverting menu item" << endl;
	OptionsMenu_SetProofMode(mw->uim,CM_PROOFMODE_NONE);
	cerr << "done" << endl;
	return(FALSE);
}


static void optionsmenu_radio_dispatch(GtkAction *act,GtkRadioAction *ra,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	enum CMProofMode proofmode=CMProofMode(gtk_radio_action_get_current_value(ra));
	cerr << "Proofmode set to: " << proofmode << endl;
	try
	{
		mw->state->profilemanager.SetProofMode(proofmode);
		mw->state->layout->FlushThumbnails();
		pp_mainwindow_refresh(mw);
	}
	catch(const char *err)
	{
		ErrorMessage_Dialog(err,GTK_WIDGET(mw));
		cerr << "Dialog displayed - adding idle function..." << endl;
		gtk_idle_add(radioidlefunc,mw);
	}
}


static GtkActionEntry optionsmenu_entries[] = {
  { "OptionsMenu", NULL, N_("_Options") },

  { "Paths", NULL, N_("_Paths..."), NULL, N_("Set search paths for ICC profiles, borders, etc."), G_CALLBACK(options_paths) },
  { "ColourManagement", NULL, N_("_Colour Management..."), NULL, N_("Set colour management options"), G_CALLBACK(options_colourmanagement) },
  { "Units", NULL, N_("_Units..."), NULL, N_("Select the units used throughout PhotoPrint"), G_CALLBACK(options_units) },
  { "Scaling", NULL, N_("_Scaling..."), NULL, N_("Select the preferred scaling method."), G_CALLBACK(options_scaling) },
};

static GtkToggleActionEntry optionsmenu_toggle_entries[] = {
  { "HighresPreviews", NULL, N_("High-res Previews"), NULL, N_("Render high-resolution previews in the background"), G_CALLBACK(options_highres), FALSE }
};

static GtkRadioActionEntry optionsmenu_radio_entries[] = {
  { "NormalDisplay", NULL, N_("_Normal Display"), NULL, N_("Display colours with no print simulation"),  CM_PROOFMODE_NONE},
  { "SimulatePrint", NULL, N_("Simulate Prin_t"), NULL, N_("Adjust colours on screen to imitate printed colours, including paper white"), CM_PROOFMODE_SIMULATEPRINT},
  { "SimulatePrintAdaptWhite", NULL, N_("Simulate Print, _Adapt White"), NULL, N_("Adjust colours to imitate printed colours, excluding paper white"), CM_PROOFMODE_SIMULATEPRINTADAPTWHITE},
};


static const char *optionsmenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='OptionsMenu'>"
"      <menuitem action='Paths'/>"
"      <menuitem action='Units'/>"
"      <menuitem action='Scaling'/>"
"      <menuitem action='ColourManagement'/>"
"      <separator/>"
"      <menuitem action='NormalDisplay'/>"
"      <menuitem action='SimulatePrint'/>"
"      <menuitem action='SimulatePrintAdaptWhite'/>"
"      <separator/>"
"      <menuitem action='HighresPreviews'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildOptionsMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("OptionsMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, optionsmenu_entries, G_N_ELEMENTS (optionsmenu_entries), userdata);
	gtk_action_group_add_toggle_actions (action_group, optionsmenu_toggle_entries, G_N_ELEMENTS (optionsmenu_toggle_entries), userdata);
	gtk_action_group_add_radio_actions (action_group, optionsmenu_radio_entries, G_N_ELEMENTS (optionsmenu_radio_entries), 0, G_CALLBACK(optionsmenu_radio_dispatch), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, optionsmenu_ui_description, -1, &error))
		throw error->message;
}


void OptionsMenu_SetHighresPreviews(GtkUIManager *ui_manager,int hrpreview)
{
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/OptionsMenu/HighresPreviews");
	if(act)
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act),hrpreview);
}


void OptionsMenu_SetProofMode(GtkUIManager *ui_manager,enum CMProofMode item)
{
	cerr << "Setting proof mode to " << item << endl;
#if 0
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/OptionsMenu/NormalDisplay");
	if(act)
		gtk_radio_action_set_current_value(GTK_RADIO_ACTION(act),item);

#else
	const char *menupaths[]=
	{
		"/MainMenu/OptionsMenu/NormalDisplay",
		"/MainMenu/OptionsMenu/SimulatePrint",
		"/MainMenu/OptionsMenu/SimulatePrintAdaptWhite"
	};

	GtkWidget *w;

	for(unsigned int i=0;i<(sizeof(menupaths)/sizeof(const char *));++i)
	{
		w=gtk_ui_manager_get_widget(ui_manager,menupaths[i]);
		if(w)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),(enum CMProofMode)i==item);
		}
	}
#endif
}
