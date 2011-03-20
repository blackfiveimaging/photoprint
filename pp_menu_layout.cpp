#include <iostream>

#include <gtk/gtkstock.h>
#include <gtk/gtkradioaction.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>

#include "pp_menu_layout.h"
#include "pp_mainwindow.h"
#include "dialogs.h"
#include "miscwidgets/generaldialogs.h"
#include "progressbar.h"
#include "debug.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum LAYOUTRADIOOPTIONS {LAYOUT_NUP,LAYOUT_SINGLE,LAYOUT_POSTER,LAYOUT_CAROUSEL};


static void layoutmenu_clearlayout(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	mw->state->layout->Clear();
	pp_mainwindow_refresh(mw);
}


static void layoutmenu_single(gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	if(strcmp(mw->state->layoutdb.FindString("LayoutType"),"Single")==0)
		return;
	mw->state->layoutdb.SetString("LayoutType","Single");
	
	if(mw->layout)
		mw->state->layout->LayoutToDB(mw->state->layoutdb);

	pp_mainwindow_rebuild(mw);
}


static void layoutmenu_nup(gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	if(strcmp(mw->state->layoutdb.FindString("LayoutType"),"NUp")==0)
		return;
	mw->state->layoutdb.SetString("LayoutType","NUp");

	if(mw->layout)
		mw->state->layout->LayoutToDB(mw->state->layoutdb);

	pp_mainwindow_rebuild(mw);
}


static void layoutmenu_carousel(gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	if(strcmp(mw->state->layoutdb.FindString("LayoutType"),"Carousel")==0)
		return;
	mw->state->layoutdb.SetString("LayoutType","Carousel");

	if(mw->layout)
		mw->state->layout->LayoutToDB(mw->state->layoutdb);

	pp_mainwindow_rebuild(mw);
}


static void layoutmenu_poster(gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	if(strcmp(mw->state->layoutdb.FindString("LayoutType"),"Poster")==0)
		return;
	mw->state->layoutdb.SetString("LayoutType","Poster");

	if(mw->layout)
		mw->state->layout->LayoutToDB(mw->state->layoutdb);

	pp_mainwindow_rebuild(mw);
}


static void layoutmenu_radio_dispatch(GtkAction *act,GtkRadioAction *ra,gpointer *ob)
{
	switch(gtk_radio_action_get_current_value(ra))
	{
		case LAYOUT_NUP:
			layoutmenu_nup(ob);
			break;
		case LAYOUT_SINGLE:
			layoutmenu_single(ob);
			break;
		case LAYOUT_CAROUSEL:
			layoutmenu_carousel(ob);
			break;
		case LAYOUT_POSTER:
			layoutmenu_poster(ob);
			break;
	}
}


static void layoutmenu_setbackground(GtkAction *act,gpointer *ob)
{
	pp_MainWindow *mw=(pp_MainWindow *)ob;
	static std::string prevfile;
	// Need to get existing filename...

	std::string bg=Background_Dialog(&mw->window,*mw->state,prevfile);

	Debug[TRACE] << "Setting background to: " << bg << endl;

	mw->state->layout->SetBackground(bg.c_str());
//	if(prevfile)
//		free(prevfile);
	prevfile=bg;
	pp_mainwindow_refresh(mw);
}




static GtkActionEntry layoutmenu_entries[] = {
  { "LayoutMenu", NULL, N_("_Layout") },
  
  { "ClearLayout", NULL, N_("_Clear Layout"), "<control>K", N_("Remove all images from the layout"), G_CALLBACK(layoutmenu_clearlayout) },
  { "SetBackground", NULL, N_("Set _Background"), "<control>B", N_("Set a background image for the current layout"), G_CALLBACK(layoutmenu_setbackground) },
};


static GtkRadioActionEntry layoutmenu_radio_entries[] = {
  { "AutoLayout", NULL, N_("_Auto Layout"), NULL, N_("Automatically place images in a grid"), LAYOUT_NUP },
  { "Poster", NULL, N_("_Poster"), NULL, N_("Print an image in multiple pages, to be assembled into a poster"), LAYOUT_POSTER },
  { "Carousel", NULL, N_("_Carousel"), NULL, N_("Fade images into a circular 'carousel' - ideal for CD labels."), LAYOUT_CAROUSEL },
  { "ManualSize", NULL, N_("_Manual Size"), NULL, N_("Print images on a single sheet, at a specified scale."), LAYOUT_SINGLE }
};


static const char *layoutmenu_ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='LayoutMenu'>"
"      <menuitem action='ClearLayout'/>"
"      <menuitem action='SetBackground'/>"
"      <separator/>"
"      <menuitem action='AutoLayout'/>"
"      <menuitem action='Poster'/>"
"      <menuitem action='Carousel'/>"
"      <menuitem action='ManualSize'/>"
"    </menu>"
"  </menubar>"
"</ui>";


void BuildLayoutMenu(void *userdata,GtkUIManager *ui_manager)
{
	GError *error=NULL;
	GtkActionGroup *action_group;
	action_group = gtk_action_group_new ("LayoutMenuActions");
	gtk_action_group_set_translation_domain(action_group,PACKAGE);
	gtk_action_group_add_actions (action_group, layoutmenu_entries, G_N_ELEMENTS (layoutmenu_entries), userdata);
	gtk_action_group_add_radio_actions (action_group, layoutmenu_radio_entries, G_N_ELEMENTS (layoutmenu_radio_entries), 0, G_CALLBACK(layoutmenu_radio_dispatch), userdata);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, layoutmenu_ui_description, -1, &error))
		throw error->message;
}


void LayoutMenu_SetLayout(GtkUIManager *ui_manager,const char *layouttype)
{
	enum LAYOUTRADIOOPTIONS item=LAYOUT_NUP;
	if(strcmp(layouttype,"Single")==0)
		item=LAYOUT_SINGLE;
	if(strcmp(layouttype,"NUp")==0)
		item=LAYOUT_NUP;
	if(strcmp(layouttype,"Poster")==0)
		item=LAYOUT_POSTER;
	if(strcmp(layouttype,"Carousel")==0)
		item=LAYOUT_CAROUSEL;

#if 0
	GtkAction *act=gtk_ui_manager_get_action(ui_manager,"/MainMenu/LayoutMenu/AutoLayout");
	if(act)
		gtk_radio_action_set_current_value(GTK_RADIO_ACTION(act),item);

#else
	const char *menupaths[]=
	{
		"/MainMenu/LayoutMenu/AutoLayout",
		"/MainMenu/LayoutMenu/ManualSize",
		"/MainMenu/LayoutMenu/Poster",
		"/MainMenu/LayoutMenu/Carousel"
	};

	GtkWidget *w;

	for(unsigned int i=0;i<(sizeof(menupaths)/sizeof(const char *));++i)
	{
		w=gtk_ui_manager_get_widget(ui_manager,menupaths[i]);
		if(w)
		{
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w),(enum LAYOUTRADIOOPTIONS)i==item);
		}
	}
#endif
}


void LayoutMenu_SetLayoutCapabilities(GtkUIManager *ui_manager,int features)
{
	struct menucapentry
	{
		const char *path;
		int flag;
	};
	
	menucapentry tags[]={
		{"/MainMenu/LayoutMenu/SetBackground",PPLAYOUT_BACKGROUND},
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
