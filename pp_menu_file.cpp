#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkmain.h>

#include "config.h"

#include "pp_menu_file.h"
#include "errordialogqueue.h"
#include "pp_mainwindow.h"
#include "dialogs.h"
#include "printpreview.h"
#include "miscwidgets/generaldialogs.h"
#include "progressbar.h"
#include "pathsupport.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;


static void savegeometry(pp_MainWindow *mw)
{
	gint x,y,w,h;
	gtk_window_get_position(GTK_WINDOW(mw),&x,&y);
	gtk_window_get_size(GTK_WINDOW(mw),&w,&h);

	mw->state->SetInt("Win_X",x);
	mw->state->SetInt("Win_Y",y);
	mw->state->SetInt("Win_W",w);
	mw->state->SetInt("Win_H",h);
}


static void file_open_preset(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	char *filename=File_Dialog(_("Open preset..."),mw->state->filename);
	mw->state->layout->FlushHRPreviews();
	if(filename)
	{
		mw->state->SetFilename(filename);
		mw->state->ParseConfigFile();
		
		g_free(filename);

		pp_mainwindow_rebuild(mw);
	}
}


static void file_save_preset(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	savegeometry(mw);
	mw->state->SaveConfigFile();
}


static void file_save_as(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	savegeometry(mw);
	char *filename=File_Save_Dialog(_("Save preset..."),mw->state->filename,GTK_WIDGET(&mw->window));

	if(filename)
	{
		mw->state->SetFilename(filename);
		mw->state->SaveConfigFile();
	
		g_free(filename);
	}
}


static void file_save_default(GtkWidget *wid,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	savegeometry(mw);
	mw->state->SetDefaultFilename();
	mw->state->SaveConfigFile();
}


static void file_export_tiff(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	ExportTiff_Dialog(GTK_WINDOW(mw),*mw->state);
}

static void file_export_jpeg(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	ExportJPEG_Dialog(GTK_WINDOW(mw),*mw->state);
}

static void file_print_setup(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;	
	PrintSetup_Dialog(GTK_WINDOW(mw),*mw->state);
	pp_mainwindow_refresh(mw);
}


static void file_print_preview(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;	
	PrintPreview_Dialog(GTK_WINDOW(mw),*mw->state);
}


static void file_print(GtkAction *action,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	ProgressBar p(_("Generating print data..."),true,GTK_WIDGET(mw));

	try
	{
		mw->state->layout->Print(&p);
	}
	catch (const char *err)
	{
		ErrorMessage_Dialog(err,GTK_WIDGET(mw));
	}
}


static void file_quit(GtkAction *action,gpointer ob)
{
	gtk_main_quit();
}


static GtkActionEntry filemenu_entries[] = {
  { "FileMenu", NULL, N_("_File") },

  { "OpenPreset", GTK_STOCK_OPEN, N_("_Open Preset..."), "<control>O", N_("Open a preset"), G_CALLBACK(file_open_preset) },
  { "SavePreset", GTK_STOCK_SAVE, N_("_Save Preset"), "<control>S", N_("Save a preset"), G_CALLBACK(file_save_preset) },
  { "SaveAs", NULL, N_("Save _As..."), NULL, N_("Save preset with a new filename"), G_CALLBACK(file_save_as) },
  { "SaveDefault", NULL, N_("Save _Default"), NULL, N_("Save preset as the default"), G_CALLBACK(file_save_default) },
  { "ExportMenu", NULL, N_("E_xport") },
  { "ExportTiff", NULL, N_("Export _TIFF..."), NULL, N_("Export pages as TIFF files"), G_CALLBACK(file_export_tiff) },
  { "ExportJPEG", NULL, N_("Export _JPEG..."), NULL, N_("Export pages as JPEG files"), G_CALLBACK(file_export_jpeg) },
  { "PrintPreview", NULL, N_("Print Pre_view..."), NULL, N_("Preview how the printed page will look"), G_CALLBACK(file_print_preview) },
  { "PrintSetup", NULL, N_("Print S_etup..."), NULL, N_("Set printer driver and options"), G_CALLBACK(file_print_setup) },
  { "Print", GTK_STOCK_PRINT, N_("_Print"), "<control>P", N_("Print pages"), G_CALLBACK(file_print) },
  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q", N_("Exit PhotoPrint"), G_CALLBACK(file_quit)},
};

static const char *filemenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='OpenPreset'/>"
"      <menuitem action='SavePreset'/>"
"      <menuitem action='SaveAs'/>"
"      <menuitem action='SaveDefault'/>"
"      <separator/>"
"      <menu action='ExportMenu'>"
"        <menuitem action='ExportTiff'/>"
"        <menuitem action='ExportJPEG'/>"
"      </menu>"
"      <separator/>"
"      <menuitem action='PrintSetup'/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='Print'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildFileMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("FileMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, filemenu_entries, G_N_ELEMENTS (filemenu_entries), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, filemenu_ui_description, -1, &error))
		throw error->message;
}
