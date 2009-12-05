#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkfilechooser.h>
#include <gtk/gtkfilechooserdialog.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkuimanager.h>

#include "stpui_widgets/stpui_combo.h"
#include "progressbar.h"
#include "support/pathsupport.h"
#include "layout.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "pixbufthumbnail/egg-pixbuf-thumbnail.h"
#include "effects/effects_dialog.h"
#include "pp_menu_file.h"
#include "pp_menu_edit.h"
#include "pp_menu_layout.h"
#include "pp_menu_image.h"
#include "pp_menu_options.h"
#include "pp_menu_shortcuts.h"
#include "pp_menu_help.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

#include "pp_mainwindow.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_mainwindow_signals[LAST_SIGNAL] = { 0 };

static void pp_mainwindow_class_init (pp_MainWindowClass *klass);
static void pp_mainwindow_init (pp_MainWindow *stpuicombo);
static void layout_changed(GtkWidget *wid,gpointer *ob);



// Functions invoked from the menus:


static void layout_selection_changed(GtkWidget *wid,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	PP_ROTATION rotation=ImageMenu_GetRotation(mw->uim);
	bool allowcropping=ImageMenu_GetCropFlag(mw->uim);
	bool removeimage=true;

	LayoutIterator it(*mw->state->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	if(ii)
	{
		allowcropping=ii->allowcropping;
		rotation=ii->rotation;
	}
	else
		removeimage=false;

	while(ii)
	{
		if(rotation!=ii->rotation)
			rotation=PP_ROTATION_NONE;
		allowcropping&=ii->allowcropping;
		ii=it.NextSelected();
	}

	ImageMenu_SetCropFlag(mw->uim,allowcropping);
	ImageMenu_SetRotation(mw->uim,rotation);
}


static void layout_popupmenu(GtkWidget *wid,gpointer *ob)
{
	layout_selection_changed(NULL,ob);
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	ImageMenu_DoPopup(mw->uim);
}


static void layout_changed(GtkWidget *wid,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;

	if(mw->layout)
		mw->state->layout->RefreshWidget(mw->layout);
}


void pp_mainwindow_refresh(pp_MainWindow *ob)
{
	if(ob->layout)
		ob->state->layout->RefreshWidget(ob->layout);

	LayoutMenu_SetLayout(ob->uim,ob->state->layout->GetType());
}


GtkWidget*
pp_mainwindow_new (PhotoPrint_State *state)
{
	pp_MainWindow *ob=PP_MAINWINDOW(g_object_new (pp_mainwindow_get_type (), NULL));

	gtk_window_set_title (GTK_WINDOW (ob), PACKAGE_STRING);
	gtk_window_set_default_size(GTK_WINDOW(ob),state->FindInt("Win_W"),state->FindInt("Win_H"));
	gtk_window_move(GTK_WINDOW(ob),state->FindInt("Win_X"),state->FindInt("Win_Y"));
	ob->state=state;

	ob->vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(ob),ob->vbox);
	gtk_widget_show(ob->vbox);



	GtkWidget *menubar;
	GtkAccelGroup *accel_group;
	ob->uim = gtk_ui_manager_new ();

	BuildFileMenu(ob,ob->uim);
	BuildEditMenu(ob,ob->uim);
	BuildLayoutMenu(ob,ob->uim);
	BuildImageMenu(ob,ob->uim);
	BuildOptionsMenu(ob,ob->uim);
	BuildShortcutsMenu(ob,ob->uim);
	BuildHelpMenu(ob,ob->uim);
	
	accel_group = gtk_ui_manager_get_accel_group (ob->uim);
	gtk_window_add_accel_group (GTK_WINDOW (ob), accel_group);
	
	GtkWidget *tmp;
	tmp=gtk_ui_manager_get_widget(ob->uim,"/MainMenu/EditMenu/Cut");
	gtk_widget_set_sensitive(tmp,false);
	tmp=gtk_ui_manager_get_widget(ob->uim,"/MainMenu/EditMenu/Copy");
	gtk_widget_set_sensitive(tmp,false);
	tmp=gtk_ui_manager_get_widget(ob->uim,"/MainMenu/EditMenu/Paste");
	gtk_widget_set_sensitive(tmp,false);

	menubar = gtk_ui_manager_get_widget (ob->uim, "/MainMenu");
	gtk_box_pack_start(GTK_BOX(ob->vbox),menubar,FALSE,TRUE,0);
	gtk_widget_show(menubar);


	if((ob->layout=state->layout->CreateWidget()))
	{
		gtk_box_pack_start(GTK_BOX(ob->vbox),ob->layout,TRUE,TRUE,0);
		gtk_widget_show(ob->layout);
		g_signal_connect(G_OBJECT(ob->layout),"changed",G_CALLBACK(layout_changed),ob);
		g_signal_connect(G_OBJECT(ob->layout),"popupmenu",G_CALLBACK(layout_popupmenu),ob);
		g_signal_connect(G_OBJECT(ob->layout),"selection_changed",G_CALLBACK(layout_selection_changed),ob);
	}

	int caps=ob->state->layout->GetCapabilities();
	ImageMenu_SetLayoutCapabilities(ob->uim,caps);
	LayoutMenu_SetLayoutCapabilities(ob->uim,caps);

	OptionsMenu_SetHighresPreviews(ob->uim,state->FindInt("HighresPreviews"));
	OptionsMenu_SetProofMode(ob->uim,CMProofMode(state->profilemanager.FindInt("ProofMode")));

	pp_mainwindow_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_mainwindow_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_mainwindow_info =
		{
			sizeof (pp_MainWindowClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_mainwindow_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_MainWindow),
			0,
			(GInstanceInitFunc) pp_mainwindow_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_WINDOW, "pp_MainWindow", &pp_mainwindow_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_mainwindow_class_init (pp_MainWindowClass *klass)
{
	pp_mainwindow_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_MainWindowClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_mainwindow_init (pp_MainWindow *ob)
{
	ob->state=NULL;
	ob->layout=NULL;
	ob->prevfile=NULL;
}


void pp_mainwindow_rebuild(pp_MainWindow *mw)
{
	try
	{
		ProgressBar p(_("Transferring images..."),true,GTK_WIDGET(mw));
		if(mw->state->NewLayout(&p))
		{
			gtk_widget_destroy(mw->layout);
	
			if((mw->layout=mw->state->layout->CreateWidget()))
			{
				gtk_box_pack_start(GTK_BOX(mw->vbox),mw->layout,TRUE,TRUE,0);
				gtk_widget_show(mw->layout);
				g_signal_connect(G_OBJECT(mw->layout),"changed",G_CALLBACK(layout_changed),mw);
				g_signal_connect(G_OBJECT(mw->layout),"popupmenu",G_CALLBACK(layout_popupmenu),mw);
				g_signal_connect(G_OBJECT(mw->layout),"selection_changed",G_CALLBACK(layout_selection_changed),mw);
			}
			mw->state->layout->FlushThumbnails();
		}
		int caps=mw->state->layout->GetCapabilities();
		ImageMenu_SetLayoutCapabilities(mw->uim,caps);
		LayoutMenu_SetLayoutCapabilities(mw->uim,caps);

		OptionsMenu_SetHighresPreviews(mw->uim,mw->state->FindInt("HighresPreviews"));
		OptionsMenu_SetProofMode(mw->uim,CMProofMode(mw->state->profilemanager.FindInt("ProofMode")));

		pp_mainwindow_refresh(mw);
	}
	catch(const char *err)
	{
		ErrorMessage_Dialog(err);
	}
}
