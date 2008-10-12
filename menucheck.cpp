#include <iostream>

#include <gutenprint/gutenprint.h>

#include <gtk/gtk.h>

#include "pp_menu_file.h"
#include "pp_menu_edit.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

#if 0
/* File Menu */

static GtkActionEntry filemenu_entries[] = {
  { "FileMenu", NULL, N_("_File") },

  { "OpenPreset", GTK_STOCK_OPEN, N_("_Open Preset..."), "<control>O", N_("Open a preset"), NULL },
  { "SavePreset", GTK_STOCK_SAVE, N_("_Save Preset"), "<control>S", N_("Save a preset"), NULL },
  { "SaveAs", NULL, N_("Save _As..."), NULL, N_("Save preset with a new filename"), NULL },
  { "SaveDefault", NULL, N_("Save _Default"), NULL, N_("Save preset as the default"), NULL },
  { "ExportTiff", NULL, N_("E_xport TIFF..."), NULL, N_("Export pages as TIFF files"), NULL },
  { "PrintPreview", NULL, N_("Print Pre_view..."), NULL, N_("Preview how the printed page will look"), NULL },
  { "PrintSetup", NULL, N_("Print S_etup..."), NULL, N_("Set printer driver and options"), NULL },
  { "Print", NULL, N_("_Print"), "<control>P", N_("Print pages"), NULL },
  { "Quit", NULL, N_("_Quit"), "<control>Q", N_("Exit PhotoPrint"), NULL },
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
"      <menuitem action='ExportTiff'/>"
"      <separator/>"
"      <menuitem action='PrintPreview'/>"
"      <menuitem action='PrintSetup'/>"
"      <menuitem action='Print'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"  </menubar>"
"</ui>";
#endif




/* Normal items */
static GtkActionEntry entries[] = {
  { "LayoutMenu", NULL, N_("_Layout") },
  { "ImageMenu", NULL, N_("_Image") },
  { "OptionsMenu", NULL, N_("_Options") },
  { "HelpMenu", NULL, N_("_Help") },

  { "ClearLayout", NULL, N_("_Clear Layout"), "<control>K", N_("Remove all images from the layout"), NULL },
  { "Set Background", NULL, N_("Set _Background"), "<control>B", N_("Set a background image for the current layout"), NULL },

  { "AddImage", NULL, N_("_Add Image..."), "<control>I", N_("Add images to the current layout"), NULL },
  { "RemoveImage", NULL, N_("_Remove Image"), NULL, N_("Remove selected images from the layout"), NULL },

  { "Rotation", NULL, N_("_Rotation") },

  { "SetImageMask", NULL, N_("Set Image _Mask..."), NULL, N_("Set a border mask for the selected image"), NULL },
  { "Effects", NULL, N_("_Effects..."), NULL, N_("Apply effects to the selected image"), NULL },
  { "SetColourProfile", NULL, N_("Set Colour _Profile..."), NULL, N_("Assign an ICC profile or custom rendering intent to the image"), NULL },

  { "Paths", NULL, N_("_Paths..."), NULL, N_("Set search paths for ICC profiles, borders, etc."), NULL },
  { "ColourManagement", NULL, N_("_Colour Management..."), NULL, N_("Set colour management options"), NULL },
  { "Units", NULL, N_("_Units..."), NULL, N_("Select the units used throughout PhotoPrint"), NULL },
  { "Scaling", NULL, N_("_Scaling..."), NULL, N_("Select the preferred scaling method."), NULL },

  { "About", NULL, N_("About..."), NULL, N_("Some information about PhotoPrint"), NULL },
};

/* Toggle items */
static GtkToggleActionEntry toggle_entries[] = {
  { "AllowCropping", NULL, N_("Allow _Cropping"), NULL, N_("Crop the selected images to fill the available space"), NULL, FALSE }
};

/* Radio items */
static GtkRadioActionEntry layout_radio_entries[] = {
  { "AutoLayout", NULL, N_("_Auto Layout"), NULL, N_("Automatically place images in a grid"), 0 },
  { "Poster", NULL, N_("_Poster"), NULL, N_("Print an image in multiple pages, to be assembled into a poster"), 1 },
  { "Carousel", NULL, N_("_Carousel"), NULL, N_("Fade images into a circular 'carousel' - ideal for CD labels."), 2 },
  { "ManualSize", NULL, N_("_Manual Size"), NULL, N_("Print images on a single sheet, at a specified scale."), 3 }
};

static GtkRadioActionEntry rotation_radio_entries[] = {
  { "RotationAuto", NULL, N_("_Auto"), NULL, N_("Automatically choose rotation to fit the available space."), 0 },
  { "RotationNone", NULL, N_("_None"), NULL, N_("No rotation."), 1 },
  { "Rotation90", NULL, N_("_90 Degrees"), NULL, N_("Rotate 90 degrees clockwise."), 2 },
  { "Rotation180", NULL, N_("_180 Degrees"), NULL, N_("Rotate 180 degrees."), 3 },
  { "Rotation270", NULL, N_("_270 Degrees"), NULL, N_("Rotate 90 degrees anticlockwise."), 4 }
};


static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='LayoutMenu'>"
"      <menuitem action='ClearLayout'/>"
"      <menuitem action='Set Background'/>"
"      <separator/>"
"      <menuitem action='AutoLayout'/>"
"      <menuitem action='Poster'/>"
"      <menuitem action='Carousel'/>"
"      <menuitem action='ManualSize'/>"
"    </menu>\n"
"    <menu action='ImageMenu'>"
"      <menuitem action='AddImage'/>"
"      <menuitem action='RemoveImage'/>"
"      <separator/>"
"      <menuitem action='AllowCropping'/>"
"      <menu action='Rotation'>"
"        <menuitem action='RotationAuto'/>"
"        <menuitem action='RotationNone'/>"
"        <menuitem action='Rotation90'/>"
"        <menuitem action='Rotation180'/>"
"        <menuitem action='Rotation270'/>"
"      </menu>"
"      <menuitem action='SetImageMask'/>"
"      <menuitem action='Effects'/>"
"      <menuitem action='SetColourProfile'/>"
"    </menu>\n"
"    <menu action='OptionsMenu'>"
"      <menuitem action='Paths'/>"
"      <menuitem action='ColourManagement'/>"
"      <menuitem action='Units'/>"
"      <menuitem action='Scaling'/>"
"    </menu>\n"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

#if 0

int main(int argc,char **argv)
{
	gtk_init(&argc,&argv);

	GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
//	GtkWidget *menu=pp_menu_new();

	GtkWidget *menubar;
	GtkActionGroup *action_group;
	GtkUIManager *ui_manager;
	GtkAccelGroup *accel_group;
	GError *error;

	ui_manager = gtk_ui_manager_new ();

//	BuildFileMenu(window,ui_manager);
	// Layout menu

	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), window);
	gtk_action_group_add_toggle_actions (action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), window);
	gtk_action_group_add_radio_actions (action_group, layout_radio_entries, G_N_ELEMENTS (layout_radio_entries), 0, NULL, window);
	gtk_action_group_add_radio_actions (action_group, rotation_radio_entries, G_N_ELEMENTS (rotation_radio_entries), 0, NULL, window);
	
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error))
		throw error->message;

	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	
	GtkWidget *tmp;
	tmp=gtk_ui_manager_get_widget(ui_manager,"/MainMenu/FileMenu/PrintPreview");
	gtk_widget_set_sensitive(tmp,false);
	tmp=gtk_ui_manager_get_widget(ui_manager,"/MainMenu/EditMenu/Cut");
	gtk_widget_set_sensitive(tmp,false);
	tmp=gtk_ui_manager_get_widget(ui_manager,"/MainMenu/EditMenu/Copy");
	gtk_widget_set_sensitive(tmp,false);
	tmp=gtk_ui_manager_get_widget(ui_manager,"/MainMenu/EditMenu/Paste");
	gtk_widget_set_sensitive(tmp,false);

	menubar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");

	gtk_container_add(GTK_CONTAINER(window),menubar);
	gtk_widget_show(menubar);
	gtk_widget_show(window);
	gtk_main();
	return(0);
}
#endif

#define TEST_STRING "Initializing..."

void check(const char *tag)
{
	cerr << tag << ": ";
	if(strcmp(_(TEST_STRING),TEST_STRING)==0)
		cerr << "Failed!" << endl;
	else
		cerr << "ok" << endl;
	setlocale(LC_ALL,"");
}

#if 1
int main(int argc,char **argv)
{
	setlocale(LC_ALL,"");
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	check("Startup");

	stp_init();
	check("stp_init()");

	cerr << "Dither Algorithm -> " << dgettext("gutenprint","Dither Algorithm") << endl;

	stp_vars_t *vars=stp_vars_create();
	check("stp_vars_create()");

	stp_set_driver(vars,"ps2");
	check("stp_set_driver()");

	stp_parameter_t desc;
	stp_describe_parameter(vars,"Brightness",&desc);
	check("stp_describe_paramater(<double>)");

	stp_describe_parameter(vars,"PageSize",&desc);
	check("stp_describe_paramater(<string>)");

	stp_set_string_parameter(vars,"PageSize","A4");
	check("stp_set_string_parameter()");

	int l,r,t,b;
	stp_get_imageable_area(vars, &l, &r, &b, &t);
	check("stp_get_imageable_area()");

	return(0);
}

#else

int main(int argc,char **argv)
{
	setlocale(LC_ALL,"");
	bindtextdomain(PACKAGE,LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	check("Startup");

	cerr << "Setting locale to 'C'" << endl;

	char *oldlocale=setlocale(LC_ALL,NULL);
    char *savedlocale=strdup(oldlocale);
	cerr << "Old locale setting: " << oldlocale << endl;

	char *result=setlocale(LC_ALL,"C");

	cerr << "Old locale setting: " << oldlocale << endl;
	cerr << "Result of setlocale: " << result << endl;

	check("Checking translation (this should fail)");

	cerr << "Restoring locale" << endl;

	setlocale(LC_ALL,savedlocale);
	free(savedlocale);

	check("Checking translation (this should succeed)");

	return(0);
}
#endif
