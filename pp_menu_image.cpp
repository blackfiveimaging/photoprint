#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkradioaction.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkfilechooser.h>
#include <gtk/gtkfilechooserdialog.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkmain.h>

#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "progressbar.h"
#include "support/pathsupport.h"
#include "support/debug.h"
#include "support/layoutrectangle.h" // For rotation enums
//#include "effects/effects_dialog.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "pp_menu_image.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

// Yuk - but it's a royal pain to do this any other way.
// Can't easily get signal connection IDs from the automatically-built menus

static bool blocksignals=false;


// Preview widget for file chooser

static void updatepreview(GtkWidget *fc,void *ud)
{
	GtkWidget *preview=GTK_WIDGET(ud);
	gchar *fn=gtk_file_chooser_get_preview_filename(GTK_FILE_CHOOSER(fc));
	bool active=false;
	if(fn)
	{
		GError *err=NULL;
		GdkPixbuf *pb=egg_pixbuf_get_thumbnail_for_file(fn,EGG_PIXBUF_THUMBNAIL_NORMAL,&err);
		if(pb)
		{
			gtk_image_set_from_pixbuf(GTK_IMAGE(preview),pb);
			g_object_unref(pb);
			active=true;		
		}	
	}
	gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(fc),active);
}


static void imagemenu_addimage(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	PhotoPrint_State *state=mw->state;
	GtkWidget *sel;

	sel = gtk_file_chooser_dialog_new (_("Open File"),
		GTK_WINDOW(GTK_WINDOW(&mw->window)),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(sel),TRUE);
	if(mw->prevfile)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(sel),mw->prevfile);
	else
	{
#ifdef WIN32
		char *dirname=substitute_homedir("$HOME\\My Documents\\My Pictures");
#else
		char *dirname=substitute_homedir("$HOME");
#endif
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(sel),dirname);	
	}
	g_free(mw->prevfile);
	mw->prevfile=NULL;

	GtkWidget *preview=gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(sel),GTK_WIDGET(preview));
	g_signal_connect(G_OBJECT(sel),"selection-changed",G_CALLBACK(updatepreview),preview);

	if (gtk_dialog_run (GTK_DIALOG (sel)) == GTK_RESPONSE_ACCEPT)
	{
		GSList *filenames=gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(sel));
		GSList *current=filenames;

		if(filenames)
		{
			int count=0;
			while(current)
			{
				++count;
				current=g_slist_next(current);
			}
	
			ProgressBar progress(_("Adding images..."),true,GTK_WIDGET(mw));
	
			current=filenames;
			int i=0;
			int lastpage=0;
			while(current)
			{
				char *fn=(char *)current->data;
	
//				pp_Menu *menu=PP_MENU(mw->menu);
				lastpage=state->layout->AddImage(fn,ImageMenu_GetCropFlag(mw->uim),ImageMenu_GetRotation(mw->uim));
	
				if(!(progress.DoProgress(i,count)))
					break;
				++i;
	
				current=g_slist_next(current);
				if(!current)
					mw->prevfile=g_strdup(fn);
				g_free(fn);
			}
			state->layout->SetCurrentPage(lastpage);
			g_slist_free(filenames);
		}
	}
	gtk_widget_destroy (sel);
	pp_mainwindow_refresh(mw);
}


static void imagemenu_remove(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		delete ii;
		ii=it.FirstSelected();
	}
	mw->state->layout->Reflow();
	pp_mainwindow_refresh(mw);
}


static void imagemenu_duplicate(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		mw->state->layout->CopyImage(ii);
		ii=it.NextSelected();
	}
	pp_mainwindow_refresh(mw);
}


static void imagemenu_duplicatetofillpage(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	int s=mw->state->layout->CountSelected();
	if(s)
	{
		int c=mw->state->layout->FreeSlots();
		if(c)
		{
			for(int i=0;i<c;)
			{
				LayoutIterator it(*mw->state->layout);
				Layout_ImageInfo *ii=it.FirstSelected();
				while(ii && i<c)
				{
					mw->state->layout->CopyImage(ii);
					ii=it.NextSelected();
					++i;
				}
			}
		}
		else
			ErrorMessage_Dialog(_("Page is already full!"),GTK_WIDGET(mw));
	}
	pp_mainwindow_refresh(mw);
}


static void imagemenu_allowcropping(GtkToggleAction *act,gpointer *ob)
{
	if(blocksignals)
		return;

	Debug[TRACE] << "Responding to AllowCropping..." << endl;
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	bool checked=gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(act));

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	if(!ii)
		mw->state->layoutdb.SetInt("AllowCropping",checked);
	while(ii)
	{
		ii->ObtainMutexShared();
		if(ii->allowcropping!=checked)
		{
			ii->ObtainMutex();
			ii->allowcropping=checked;
			ii->ReleaseMutex();
		}
		ii->ReleaseMutex();
		ii=it.NextSelected();
	}
	pp_mainwindow_refresh(mw);
}


static void imagemenu_setmask(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	static char *prevfile=NULL;
	// FIXME: Need to get existing filename...

	char *mask=ImageMask_Dialog(&mw->window,*mw->state,prevfile);

	if(mask)
		Debug[TRACE] << "Selected " << mask << endl;

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		ii->ObtainMutex();
		ii->SetMask(mask);
		ii->ReleaseMutex();
		ii=it.NextSelected();
	}
//	if(prevfile)
//		free(prevfile);
	prevfile=mask;
	pp_mainwindow_refresh(mw);
}


static void imagemenu_setcolourprofile(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	if(ii)
	{
		ii->ObtainMutex();
		SetCustomProfileDialog(&mw->window,*mw->state,ii);
		ii->ReleaseMutex();
		pp_mainwindow_refresh(mw);
	}
	else
		ErrorMessage_Dialog(_("Please select an image first!"),GTK_WIDGET(mw));
}


static void imagemenu_radio_dispatch(GtkAction *act,GtkRadioAction *ra,gpointer *ob)
{
	if(blocksignals)
		return;

	pp_MainWindow *mw=(pp_MainWindow *)ob;
	enum PP_ROTATION rotation=PP_ROTATION(gtk_radio_action_get_current_value(ra));

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	if(!ii)
		mw->state->layoutdb.SetInt("Rotation",rotation);
	while(ii)
	{
		ii->ObtainMutexShared();
		if(ii->rotation!=rotation)
		{
			ii->ObtainMutex();
			ii->rotation=rotation;
			ii->ReleaseMutex();
		}
		ii->ReleaseMutex();
		ii=it.NextSelected();
	}
	pp_mainwindow_refresh(mw);
}


static GtkActionEntry imagemenu_entries[] = {
  { "ImageMenu", NULL, N_("_Image") },

  { "AddImage", NULL, N_("_Add Image..."), "<control>I", N_("Add images to the current layout"), G_CALLBACK(imagemenu_addimage) },
  { "RemoveImage", NULL, N_("_Remove Image"), NULL, N_("Remove selected images from the layout"), G_CALLBACK(imagemenu_remove) },
  { "DuplicateImage", NULL, N_("_Duplicate Image"), NULL, N_("Duplicate the currently selected image"), G_CALLBACK(imagemenu_duplicate) },
  { "DuplicateToFillPage", NULL, N_("Duplicate to _Fill Page"), NULL, N_("Fill the page with copies of the currently selected image"), G_CALLBACK(imagemenu_duplicatetofillpage) },

  { "Rotation", NULL, N_("_Rotation") },

  { "SetImageMask", NULL, N_("Set Image _Border..."), NULL, N_("Set a border mask for the selected image"), G_CALLBACK(imagemenu_setmask) },
//  { "Effects", NULL, N_("_Effects..."), NULL, N_("Apply effects to the selected image"), G_CALLBACK(imagemenu_seteffects) },
  { "SetColourProfile", NULL, N_("Set Colour _Profile..."), NULL, N_("Assign an ICC profile or custom rendering intent to the image"), G_CALLBACK(imagemenu_setcolourprofile) },
};


static GtkToggleActionEntry imagemenu_toggle_entries[] = {
  { "AllowCropping", NULL, N_("Allow _Cropping"), NULL, N_("Crop the selected images to fill the available space"), G_CALLBACK(imagemenu_allowcropping), FALSE }
};


static GtkRadioActionEntry imagemenu_radio_entries[] = {
  { "RotationAuto", NULL, N_("_Auto"), NULL, N_("Automatically choose rotation to fit the available space."), PP_ROTATION_AUTO },
  { "RotationNone", NULL, N_("_None"), NULL, N_("No rotation."), PP_ROTATION_NONE },
  { "Rotation90", NULL, N_("_90 Degrees"), NULL, N_("Rotate 90 degrees clockwise."), PP_ROTATION_90 },
  { "Rotation180", NULL, N_("_180 Degrees"), NULL, N_("Rotate 180 degrees."), PP_ROTATION_180 },
  { "Rotation270", NULL, N_("_270 Degrees"), NULL, N_("Rotate 90 degrees anticlockwise."), PP_ROTATION_270 }
};


static const char *imagemenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='ImageMenu'>"
"      <menuitem action='AddImage'/>"
"      <menuitem action='RemoveImage'/>"
"      <menuitem action='DuplicateImage'/>"
"      <menuitem action='DuplicateToFillPage'/>"
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
//"      <menuitem action='Effects'/>"
"      <menuitem action='SetColourProfile'/>"
"    </menu>"
"  </menubar>"
"  <popup>"
"    <menuitem action='AddImage'/>"
"    <menuitem action='RemoveImage'/>"
"    <menuitem action='DuplicateImage'/>"
"    <menuitem action='DuplicateToFillPage'/>"
"    <separator/>"
"    <menuitem action='AllowCropping'/>"
"    <menu action='Rotation'>"
"      <menuitem action='RotationAuto'/>"
"      <menuitem action='RotationNone'/>"
"      <menuitem action='Rotation90'/>"
"      <menuitem action='Rotation180'/>"
"      <menuitem action='Rotation270'/>"
"    </menu>"
"    <menuitem action='SetImageMask'/>"
//"    <menuitem action='Effects'/>"
"    <menuitem action='SetColourProfile'/>"
"  </popup>"
"</ui>";


void BuildImageMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("ImageMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, imagemenu_entries, G_N_ELEMENTS (imagemenu_entries), userdata);
	gtk_action_group_add_toggle_actions (action_group, imagemenu_toggle_entries, G_N_ELEMENTS (imagemenu_toggle_entries), userdata);
	gtk_action_group_add_radio_actions (action_group, imagemenu_radio_entries, G_N_ELEMENTS (imagemenu_radio_entries), 0, G_CALLBACK(imagemenu_radio_dispatch), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, imagemenu_ui_description, -1, &error))
		throw error->message;

	pp_MainWindow *mw=(pp_MainWindow *)userdata;
	bool allowcropping=mw->state->layoutdb.FindInt("AllowCropping");
	enum PP_ROTATION rotation=PP_ROTATION(mw->state->layoutdb.FindInt("Rotation"));
	ImageMenu_SetCropFlag(ui_manager,allowcropping);
	ImageMenu_SetRotation(ui_manager,rotation);
}


bool ImageMenu_GetCropFlag(GtkUIManager *ui_manager)
{
	bool result=false;
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/ImageMenu/AllowCropping");
	if(act)
		result=gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(act));
	return(result);
}


void ImageMenu_SetCropFlag(GtkUIManager *ui_manager,bool active)
{
	blocksignals=true;
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/ImageMenu/AllowCropping");
	if(act)
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(act),active);
	blocksignals=false;
}


enum PP_ROTATION ImageMenu_GetRotation(GtkUIManager *ui_manager)
{
	enum PP_ROTATION result=PP_ROTATION_AUTO;
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/ImageMenu/Rotation/RotationAuto");
	if(act)
		result=PP_ROTATION(gtk_radio_action_get_current_value(GTK_RADIO_ACTION(act)));
	return(result);
}


void ImageMenu_SetRotation(GtkUIManager *ui_manager,enum PP_ROTATION rotation)
{
	blocksignals=true;
#if 0
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/ImageMenu/Rotation/RotationAuto");
	if(act)
		gtk_radio_action_set_current_value(GTK_RADIO_ACTION(act),rotation);
#else
	const char *menupaths[]=
	{
		"/MainMenu/ImageMenu/Rotation/RotationAuto",
		"/MainMenu/ImageMenu/Rotation/RotationNone",
		"/MainMenu/ImageMenu/Rotation/Rotation90",
		"/MainMenu/ImageMenu/Rotation/Rotation180",
		"/MainMenu/ImageMenu/Rotation/Rotation270"
	};

	GtkWidget *w;

	for(unsigned int i=0;i<(sizeof(menupaths)/sizeof(const char *));++i)
	{
		w=gtk_ui_manager_get_widget(ui_manager,menupaths[i]);
		if(w)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),PP_ROTATION(i)==rotation);
	}
#endif
	blocksignals=false;
}

void ImageMenu_SetLayoutCapabilities(GtkUIManager *ui_manager,int features)
{
	struct menucapentry
	{
		const char *path;
		int flag;
	};
	
	menucapentry tags[]={
		{"/MainMenu/ImageMenu/DuplicateToFillPage",PPLAYOUT_DUPLICATE},
		{"/MainMenu/ImageMenu/AllowCropping",PPLAYOUT_CROP},
		{"/MainMenu/ImageMenu/Rotation/RotationAuto",PPLAYOUT_ROTATE},
		{"/MainMenu/ImageMenu/Rotation/RotationNone",PPLAYOUT_ROTATE},
		{"/MainMenu/ImageMenu/Rotation/Rotation90",PPLAYOUT_ROTATE},
		{"/MainMenu/ImageMenu/Rotation/Rotation180",PPLAYOUT_ROTATE},
		{"/MainMenu/ImageMenu/Rotation/Rotation270",PPLAYOUT_ROTATE},
		{"/MainMenu/ImageMenu/SetImageMask",PPLAYOUT_MASK},
		{"/MainMenu/ImageMenu/SetEffects",PPLAYOUT_EFFECTS},
		{"/MainMenu/ImageMenu/SetColourProfile",PPLAYOUT_PROFILE},
		{"/popup/DuplicateToFillPage",PPLAYOUT_DUPLICATE},
		{"/popup/AllowCropping",PPLAYOUT_CROP},
		{"/popup/Rotation/RotationAuto",PPLAYOUT_ROTATE},
		{"/popup/Rotation/RotationNone",PPLAYOUT_ROTATE},
		{"/popup/Rotation/Rotation90",PPLAYOUT_ROTATE},
		{"/popup/Rotation/Rotation180",PPLAYOUT_ROTATE},
		{"/popup/Rotation/Rotation270",PPLAYOUT_ROTATE},
		{"/popup/SetImageMask",PPLAYOUT_MASK},
//		{"/popup/SetEffects",PPLAYOUT_EFFECTS},
		{"/popup/SetColourProfile",PPLAYOUT_PROFILE},
		{NULL,0}
	};

	menucapentry *p=&tags[0];
	while(p->flag)
	{
		GtkWidget *w=gtk_ui_manager_get_widget(ui_manager,p->path);
		if(w)
			gtk_widget_set_sensitive(w,(features&p->flag)!=0);
		++p;
	}
}


void ImageMenu_DoPopup(GtkUIManager *ui_manager)
{
	GtkWidget *menu=gtk_ui_manager_get_widget(ui_manager,"/popup");
	gtk_menu_popup(GTK_MENU(menu),NULL,NULL,NULL,NULL,3,gtk_get_current_event_time());
}
