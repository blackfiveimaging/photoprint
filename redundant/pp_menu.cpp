/*
 * pp_menu.cpp - provides a widget containg menus.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 * NOW REDUNDANT
 *
 *
 */

#include <iostream>
using namespace std;

#include <string.h>

#include <gtk/gtk.h>

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

#include "pp_menu.h"
#include "layout.h"

enum pp_menu_items
{
	PP_FILE_NONE,
	PP_FILE_OPEN,
	PP_FILE_SAVE,
	PP_FILE_SAVEAS,
	PP_FILE_SAVEDEFAULT,

	PP_FILE_EXPORTTIFF,
	PP_FILE_PRINTPREVIEW,
	PP_FILE_PRINTSETUP,
	PP_FILE_PRINT,
	PP_FILE_QUIT,

	PP_EDIT_SELECT_NONE,
	PP_EDIT_SELECT_ALL,
	PP_EDIT_CUT,
	PP_EDIT_COPY,
	PP_EDIT_PASTE,

	PP_LAYOUT_ADDIMAGE,
	PP_LAYOUT_CLEARLAYOUT,
	PP_LAYOUT_SETBACKGROUND,
	PP_LAYOUT_SINGLE,
	PP_LAYOUT_MULTIPLE,

	PP_LAYOUT_POSTER,
	PP_LAYOUT_CAROUSEL,
	PP_IMAGE_REMOVEIMAGE,
	PP_IMAGE_ALLOWCROPPING,
	PP_IMAGE_ROTATION_NONE,

	PP_IMAGE_ROTATION_AUTO,
	PP_IMAGE_ROTATION_90,
	PP_IMAGE_ROTATION_180,
	PP_IMAGE_ROTATION_270,
	PP_IMAGE_SETMASK,

	PP_IMAGE_SETEFFECTS,
	PP_IMAGE_SETCOLOURPROFILE,
	PP_OPTIONS_PATHS,
	PP_OPTIONS_COLOURMANAGEMENT,
	PP_OPTIONS_UNITS,

	PP_OPTIONS_SCALING,
	PP_HELP_ABOUT
};


enum {
	CHANGED_SIGNAL,
	OPENPRESET_SIGNAL,
	SAVEPRESET_SIGNAL,
	SAVEAS_SIGNAL,
	SAVEDEFAULT_SIGNAL,
	EXPORTTIFF_SIGNAL,
	PRINTPREVIEW_SIGNAL,
	PRINTSETUP_SIGNAL,
	PRINT_SIGNAL,
	SELECTNONE_SIGNAL,
	SELECTALL_SIGNAL,
	CUT_SIGNAL,
	COPY_SIGNAL,
	PASTE_SIGNAL,
	ADDIMAGE_SIGNAL,
	CLEARLAYOUT_SIGNAL,
	SETBACKGROUND_SIGNAL,
	LAYOUTSINGLE_SIGNAL,
	LAYOUTMULTIPLE_SIGNAL,
	LAYOUTPOSTER_SIGNAL,
	LAYOUTCAROUSEL_SIGNAL,
	IMAGEREMOVE_SIGNAL,
	IMAGEALLOWCROPPING_SIGNAL,
	IMAGEROTATION_SIGNAL,
	IMAGESETMASK_SIGNAL,
	IMAGEEFFECTS_SIGNAL,
	IMAGESETCOLOURPROFILE_SIGNAL,
	PATHS_SIGNAL,
	COLOURMANAGEMENT_SIGNAL,
	UNITS_SIGNAL,
	SCALING_SIGNAL,
	HELPABOUT_SIGNAL,
	LAST_SIGNAL
};

static guint pp_menu_signals[LAST_SIGNAL] = { 0 };

static void pp_menu_class_init (pp_MenuClass *klass);
static void pp_menu_init (pp_Menu *stpuicombo);


static void pp_menu_dispatcher(gpointer cbdata, guint cbaction, GtkWidget *menu_item )
{
	pp_Menu *ob=(pp_Menu *)cbdata;
	GtkCheckMenuItem *cmi;

	if(!ob->active)
		return;

	int dummy=0;

#define HANDLEITEM(item,signal) \
	case item: \
		g_signal_emit(G_OBJECT(ob),pp_menu_signals[signal],0); \
		break 
#define HANDLECHECK(item,signal,var) \
	case item: \
		cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,item)); \
		var=gtk_check_menu_item_get_active(cmi); \
		g_signal_emit(G_OBJECT (ob),pp_menu_signals[signal], 0); \
		break
#define HANDLERADIO(item,signal,var,value) \
	case item: \
		cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,item)); \
		if(gtk_check_menu_item_get_active(cmi)) \
		{ \
			var=value; \
			g_signal_emit(G_OBJECT (ob),pp_menu_signals[signal], 0); \
		} \
		break
		
	switch((enum pp_menu_items)cbaction)
	{
		case PP_FILE_QUIT:
			gtk_main_quit();
			break;
		HANDLEITEM(PP_FILE_OPEN,OPENPRESET_SIGNAL);
		HANDLEITEM(PP_FILE_SAVE,SAVEPRESET_SIGNAL);
		HANDLEITEM(PP_FILE_SAVEAS,SAVEAS_SIGNAL);
		HANDLEITEM(PP_FILE_SAVEDEFAULT,SAVEDEFAULT_SIGNAL);

		HANDLEITEM(PP_FILE_EXPORTTIFF,EXPORTTIFF_SIGNAL);
		HANDLEITEM(PP_FILE_PRINTPREVIEW,PRINTPREVIEW_SIGNAL);
		HANDLEITEM(PP_FILE_PRINTSETUP,PRINTSETUP_SIGNAL);
		HANDLEITEM(PP_FILE_PRINT,PRINT_SIGNAL);

		HANDLEITEM(PP_EDIT_SELECT_NONE,SELECTNONE_SIGNAL);
		HANDLEITEM(PP_EDIT_SELECT_ALL,SELECTALL_SIGNAL);
		HANDLEITEM(PP_EDIT_CUT,CUT_SIGNAL);
		HANDLEITEM(PP_EDIT_COPY,COPY_SIGNAL);
		HANDLEITEM(PP_EDIT_PASTE,PASTE_SIGNAL);

		HANDLEITEM(PP_LAYOUT_ADDIMAGE,ADDIMAGE_SIGNAL);
		HANDLEITEM(PP_LAYOUT_CLEARLAYOUT,CLEARLAYOUT_SIGNAL);
		HANDLEITEM(PP_LAYOUT_SETBACKGROUND,SETBACKGROUND_SIGNAL);

		HANDLERADIO(PP_LAYOUT_SINGLE,LAYOUTSINGLE_SIGNAL,dummy,0);
		HANDLERADIO(PP_LAYOUT_CAROUSEL,LAYOUTCAROUSEL_SIGNAL,dummy,0);
		HANDLERADIO(PP_LAYOUT_POSTER,LAYOUTPOSTER_SIGNAL,dummy,0);
		HANDLERADIO(PP_LAYOUT_MULTIPLE,LAYOUTMULTIPLE_SIGNAL,dummy,0);

		HANDLEITEM(PP_IMAGE_REMOVEIMAGE,IMAGEREMOVE_SIGNAL);
		HANDLECHECK(PP_IMAGE_ALLOWCROPPING,IMAGEALLOWCROPPING_SIGNAL,ob->allowcropping);
		HANDLERADIO(PP_IMAGE_ROTATION_NONE,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_NONE);
		HANDLERADIO(PP_IMAGE_ROTATION_AUTO,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_AUTO);
		HANDLERADIO(PP_IMAGE_ROTATION_90,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_90);
		HANDLERADIO(PP_IMAGE_ROTATION_180,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_180);
		HANDLERADIO(PP_IMAGE_ROTATION_270,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_270);
		HANDLEITEM(PP_IMAGE_SETMASK,IMAGESETMASK_SIGNAL);
		HANDLEITEM(PP_IMAGE_SETEFFECTS,IMAGEEFFECTS_SIGNAL);
		HANDLEITEM(PP_IMAGE_SETCOLOURPROFILE,IMAGESETCOLOURPROFILE_SIGNAL);

		HANDLEITEM(PP_OPTIONS_PATHS,PATHS_SIGNAL);
		HANDLEITEM(PP_OPTIONS_COLOURMANAGEMENT,COLOURMANAGEMENT_SIGNAL);
		HANDLEITEM(PP_OPTIONS_UNITS,UNITS_SIGNAL);
		HANDLEITEM(PP_OPTIONS_SCALING,SCALING_SIGNAL);

		HANDLEITEM(PP_HELP_ABOUT,HELPABOUT_SIGNAL);

		default:
			g_message ("Dispatching menu item: %d\n",cbaction);
			break;
	}
#undef HANDLEITEM
#undef HANDLECHECK
#undef HANDLERADIO
}


static void pp_popupmenu_dispatcher(gpointer cbdata, guint cbaction, GtkWidget *menu_item )
{
	pp_Menu *ob=(pp_Menu *)cbdata;
	GtkCheckMenuItem *cmi;

	if(!ob->active)
		return;

#define HANDLEITEM(item,signal) \
	case item: \
		g_signal_emit(G_OBJECT(ob),pp_menu_signals[signal],0); \
		break 
#define HANDLECHECK(item,signal,var) \
	case item: \
		cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->popupitemfactory,item)); \
		var=gtk_check_menu_item_get_active(cmi); \
		g_signal_emit(G_OBJECT (ob),pp_menu_signals[signal], 0); \
		break
#define HANDLERADIO(item,signal,var,value) \
	case item: \
		cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->popupitemfactory,item)); \
		if(gtk_check_menu_item_get_active(cmi)) \
		{ \
			var=value; \
			g_signal_emit(G_OBJECT (ob),pp_menu_signals[signal], 0); \
		} \
		break
		
	switch((enum pp_menu_items)cbaction)
	{
		HANDLEITEM(PP_IMAGE_REMOVEIMAGE,IMAGEREMOVE_SIGNAL);
		HANDLECHECK(PP_IMAGE_ALLOWCROPPING,IMAGEALLOWCROPPING_SIGNAL,ob->allowcropping);
		HANDLERADIO(PP_IMAGE_ROTATION_NONE,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_NONE);
		HANDLERADIO(PP_IMAGE_ROTATION_AUTO,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_AUTO);
		HANDLERADIO(PP_IMAGE_ROTATION_90,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_90);
		HANDLERADIO(PP_IMAGE_ROTATION_180,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_180);
		HANDLERADIO(PP_IMAGE_ROTATION_270,IMAGEROTATION_SIGNAL,ob->rotation,PP_ROTATION_270);
		HANDLEITEM(PP_IMAGE_SETMASK,IMAGESETMASK_SIGNAL);
		HANDLEITEM(PP_IMAGE_SETEFFECTS,IMAGEEFFECTS_SIGNAL);
		HANDLEITEM(PP_IMAGE_SETCOLOURPROFILE,IMAGESETCOLOURPROFILE_SIGNAL);

		default:
			g_message ("Dispatching popupmenu item: %d\n",cbaction);
			break;
	}
#undef HANDLEITEM
#undef HANDLECHECK
#undef HANDLERADIO
}


static GtkItemFactoryEntry menu_items[] = {
	{ _("/_File"),						NULL,
		NULL,           0, "<Branch>" },
	{ _("/File/_Open Preset..."),		"<control>O",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_OPEN, "<StockItem>", GTK_STOCK_OPEN },
	{ _("/File/_Save Preset"),			"<control>S",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_SAVE, "<StockItem>", GTK_STOCK_SAVE },
	{ N_("/File/Save _As..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_SAVEAS,	"<Item>" },
	{ N_("/File/Save _Default"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_SAVEDEFAULT,	"<Item>" },
	{ N_("/File/sep"),     				NULL,
		NULL,	0,	"<Separator>" },
	{ N_("/File/E_xport TIFF..."),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_EXPORTTIFF,	"<Item>" },
	{ N_("/File/sep"),     				NULL,
		NULL,	0,	"<Separator>" },
	{ N_("/File/Print Pre_view..."),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_PRINTPREVIEW,	"<Item>" },
	{ N_("/File/Print S_etup..."),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_PRINTSETUP,	"<Item>" },
	{ N_("/File/_Print"),				"<CTRL>P",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_PRINT,	"<StockItem>", GTK_STOCK_PRINT },
	{ N_("/File/sep"),     				NULL,
		NULL,	0,	"<Separator>" },
	{ N_("/File/_Quit"),				"<CTRL>Q",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_FILE_QUIT, "<StockItem>", GTK_STOCK_QUIT },

	{ N_("/Edit/Select _All"),			"<CTRL>A",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_EDIT_SELECT_ALL, "<Item>" },
	{ N_("/Edit/Select _None"),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_EDIT_SELECT_NONE, "<Item>" },
	{ N_("/Edit/Cut"),				"<CTRL>X",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_EDIT_CUT, "<StockItem>", GTK_STOCK_CUT },
	{ N_("/Edit/Copy"),				"<CTRL>C",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_EDIT_COPY, "<StockItem>", GTK_STOCK_COPY },
	{ N_("/Edit/Paste"),			"<CTRL>V",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_EDIT_PASTE, "<StockItem>", GTK_STOCK_PASTE },

	{ N_("/_Layout"),					NULL,
		NULL,	0,	"<Branch>" },
	{ N_("/Layout/_Clear Layout"),			"<CTRL>K",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_CLEARLAYOUT,	"<Item>" },
	{ N_("/Layout/Set _Background"),		"<CTRL>B",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_SETBACKGROUND,	"<Item>" },
	{ N_("/Layout/sep"),					NULL,
		NULL,	0,	"<Separator>" },

	{ N_("/Image/_Add Image"),			"<CTRL>I",
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_ADDIMAGE,	"<Item>" },
	{ N_("/Image/_Remove Image"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_REMOVEIMAGE,	"<Item>" },
	{ N_("/Image/sep"),					NULL,
		NULL,	0,	"<Separator>" },
	{ N_("/Image/Allow _Cropping"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ALLOWCROPPING, "<CheckItem>" },
	{ N_("/Image/Rotation/_Auto"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ROTATION_AUTO, "<RadioItem>" },
	{ N_("/Image/Rotation/_None"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ROTATION_NONE, "/Image/Rotation/Auto" },
	{ N_("/Image/Rotation/_90 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ROTATION_90, "/Image/Rotation/Auto" },
	{ N_("/Image/Rotation/_180 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ROTATION_180, "/Image/Rotation/Auto" },
	{ N_("/Image/Rotation/_270 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_ROTATION_270, "/Image/Rotation/Auto" },
	{ N_("/Image/Set image _mask..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_SETMASK,	"<Item>" },
	{ N_("/Image/_Effects..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_SETEFFECTS,	"<Item>" },
	{ N_("/Image/Set colour _profile..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_IMAGE_SETCOLOURPROFILE,	"<Item>" },

	{ N_("/Layout/_Auto Layout"),	NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_MULTIPLE, "<RadioItem>" },
	{ N_("/Layout/_Poster"),	NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_POSTER,	"/Layout/Auto Layout" },
	{ N_("/Layout/_Carousel"),	NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_CAROUSEL,	"/Layout/Auto Layout" },
	{ N_("/Layout/_Manual Size"),		NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_LAYOUT_SINGLE, "/Layout/Auto Layout" },

	{ N_("/_Options"),					NULL,
		NULL,	0,	"<Branch>" },
	{ N_("/Options/_Paths..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_OPTIONS_PATHS,	"<Item>" },
	{ N_("/Options/Colour _Management..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_OPTIONS_COLOURMANAGEMENT,	"<Item>" },
	{ N_("/Options/_Units..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_OPTIONS_UNITS,	"<Item>" },
	{ N_("/Options/_Scaling..."),			NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_OPTIONS_SCALING,	"<Item>" },

	{ N_("/_Help"),						NULL,
		NULL,	0,	"<LastBranch>" },
	{ N_("/_Help/About"),				NULL,
		(GtkItemFactoryCallback)pp_menu_dispatcher,	PP_HELP_ABOUT,	"<Item>" },
};

static gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);


static GtkItemFactoryEntry popupmenu_items[] = {
	{ N_("/_Remove Image"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_REMOVEIMAGE,	"<Item>" },
	{ "/sep",					NULL,
		NULL,	0,	"<Separator>" },
	{ N_("/Allow _Cropping"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ALLOWCROPPING, "<CheckItem>" },
	{ N_("/Rotation/Auto"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ROTATION_AUTO, "<RadioItem>" },
	{ N_("/Rotation/None"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ROTATION_NONE, "/Rotation/Auto" },
	{ N_("/Rotation/90 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ROTATION_90, "/Rotation/Auto" },
	{ N_("/Rotation/180 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ROTATION_180, "/Rotation/Auto" },
	{ N_("/Rotation/270 degrees"),		NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_ROTATION_270, "/Rotation/Auto" },
	{ N_("/Set image mask..."),			NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_SETMASK,	"<Item>" },
	{ N_("/_Effects..."),			NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_SETEFFECTS,	"<Item>" },
	{ N_("/Set colour _profile..."),			NULL,
		(GtkItemFactoryCallback)pp_popupmenu_dispatcher,	PP_IMAGE_SETCOLOURPROFILE,	"<Item>" },
};

static gint npopupmenu_items = sizeof (popupmenu_items) / sizeof (popupmenu_items[0]);


GtkWidget*
pp_menu_new ()
{
	pp_Menu *ob=PP_MENU(g_object_new (pp_menu_get_type (), NULL));

	ob->accels = gtk_accel_group_new ();
	ob->itemfactory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<PhotoPrintMenu>", ob->accels);
	gtk_item_factory_create_items (ob->itemfactory, nmenu_items, menu_items, ob);
	ob->menubar = gtk_item_factory_get_widget (ob->itemfactory, "<PhotoPrintMenu>");

	ob->popupaccels = gtk_accel_group_new ();
	ob->popupitemfactory = gtk_item_factory_new (GTK_TYPE_MENU, "<ImagePopupMenu>", ob->popupaccels);
	gtk_item_factory_create_items (ob->popupitemfactory, npopupmenu_items, popupmenu_items, ob);
	ob->popupmenubar = gtk_item_factory_get_widget (ob->popupitemfactory, "<ImagePopupMenu>");

	GtkWidget *w;
	w=gtk_item_factory_get_widget_by_action(ob->itemfactory,PP_FILE_PRINTPREVIEW);
	gtk_widget_set_sensitive(w,false);

	w=gtk_item_factory_get_widget_by_action(ob->itemfactory,PP_EDIT_CUT);
	gtk_widget_set_sensitive(w,false);

	w=gtk_item_factory_get_widget_by_action(ob->itemfactory,PP_EDIT_COPY);
	gtk_widget_set_sensitive(w,false);

	w=gtk_item_factory_get_widget_by_action(ob->itemfactory,PP_EDIT_PASTE);
	gtk_widget_set_sensitive(w,false);


	gtk_box_pack_start (GTK_BOX (ob), ob->menubar, TRUE, TRUE, 0);
	gtk_widget_show(ob->menubar);

	ob->active=false;

	return(GTK_WIDGET(ob));
}


GType
pp_menu_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_menu_info =
		{
			sizeof (pp_MenuClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_menu_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Menu),
			0,
			(GInstanceInitFunc) pp_menu_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "pp_Menu", &pp_menu_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_menu_class_init (pp_MenuClass *klass)
{
	#define DEFINE_SIGNAL(sig,str) pp_menu_signals[sig] = \
	g_signal_new (str, \
		G_TYPE_FROM_CLASS (klass), \
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), \
		G_STRUCT_OFFSET (pp_MenuClass, changed), \
		NULL, NULL, \
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	DEFINE_SIGNAL(CHANGED_SIGNAL,"changed");
	DEFINE_SIGNAL(OPENPRESET_SIGNAL,"file_openpreset");
	DEFINE_SIGNAL(SAVEPRESET_SIGNAL,"file_savepreset");
	DEFINE_SIGNAL(SAVEAS_SIGNAL,"file_saveas");
	DEFINE_SIGNAL(SAVEDEFAULT_SIGNAL,"file_savedefault");
	DEFINE_SIGNAL(EXPORTTIFF_SIGNAL,"file_exporttiff");
	DEFINE_SIGNAL(PRINTPREVIEW_SIGNAL,"file_printpreview");
	DEFINE_SIGNAL(PRINTSETUP_SIGNAL,"file_printsetup");
	DEFINE_SIGNAL(PRINT_SIGNAL,"file_print");
	
	DEFINE_SIGNAL(SELECTNONE_SIGNAL,"edit_selectnone");
	DEFINE_SIGNAL(SELECTALL_SIGNAL,"edit_selectall");
	DEFINE_SIGNAL(CUT_SIGNAL,"edit_cut");
	DEFINE_SIGNAL(COPY_SIGNAL,"edit_copy");
	DEFINE_SIGNAL(PASTE_SIGNAL,"edit_paste");

	DEFINE_SIGNAL(ADDIMAGE_SIGNAL,"image_addimage");
	DEFINE_SIGNAL(CLEARLAYOUT_SIGNAL,"image_clearlayout");
	DEFINE_SIGNAL(SETBACKGROUND_SIGNAL,"layout_setbackground");
	DEFINE_SIGNAL(LAYOUTSINGLE_SIGNAL,"layout_single");
	DEFINE_SIGNAL(LAYOUTMULTIPLE_SIGNAL,"layout_multiple");
	DEFINE_SIGNAL(LAYOUTCAROUSEL_SIGNAL,"layout_carousel");
	DEFINE_SIGNAL(LAYOUTPOSTER_SIGNAL,"layout_poster");

	DEFINE_SIGNAL(IMAGEREMOVE_SIGNAL,"image_remove");
	DEFINE_SIGNAL(IMAGEALLOWCROPPING_SIGNAL,"image_allowcropping");
	DEFINE_SIGNAL(IMAGEROTATION_SIGNAL,"image_rotation");
	DEFINE_SIGNAL(IMAGESETMASK_SIGNAL,"image_setmask");
	DEFINE_SIGNAL(IMAGEEFFECTS_SIGNAL,"image_seteffects");
	DEFINE_SIGNAL(IMAGESETCOLOURPROFILE_SIGNAL,"image_setcolourprofile");

	DEFINE_SIGNAL(PATHS_SIGNAL,"options_paths");
	DEFINE_SIGNAL(COLOURMANAGEMENT_SIGNAL,"options_colourmanagement");
	DEFINE_SIGNAL(UNITS_SIGNAL,"options_units");
	DEFINE_SIGNAL(SCALING_SIGNAL,"options_scaling");

	DEFINE_SIGNAL(HELPABOUT_SIGNAL,"help_about");
}


static void
pp_menu_init (pp_Menu *ob)
{
}


void pp_menu_refresh(pp_Menu *ob)
{
	ob->active=false;

	GtkCheckMenuItem *cmi;
	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,PP_IMAGE_ALLOWCROPPING));
	gtk_check_menu_item_set_active(cmi,ob->allowcropping);
	
	ob->active=true;
}


void pp_menu_set_layout(pp_Menu *ob,const char *type)
{
	GtkCheckMenuItem *single,*multiple,*poster,*carousel;

	ob->active=false;

	single=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,
		PP_LAYOUT_SINGLE));
	multiple=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,
		PP_LAYOUT_MULTIPLE));
	poster=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,
		PP_LAYOUT_POSTER));
	carousel=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(ob->itemfactory,
		PP_LAYOUT_CAROUSEL));

	gtk_check_menu_item_set_active(single,strcmp(type,"Single")==0);
	gtk_check_menu_item_set_active(multiple,strcmp(type,"NUp")==0);
	gtk_check_menu_item_set_active(poster,strcmp(type,"Poster")==0);
	gtk_check_menu_item_set_active(carousel,strcmp(type,"Carousel")==0);

	ob->active=true;
}


GtkAccelGroup *pp_menu_get_accels(pp_Menu *menu)
{
	return(menu->accels);
}


void pp_menu_set_layout_capabilities(pp_Menu *menu,int features)
{
	int tags[]={
		PP_IMAGE_ALLOWCROPPING,PPLAYOUT_CROP,
		PP_IMAGE_ROTATION_AUTO,PPLAYOUT_ROTATE,
		PP_IMAGE_ROTATION_NONE,PPLAYOUT_ROTATE,
		PP_IMAGE_ROTATION_90,PPLAYOUT_ROTATE,
		PP_IMAGE_ROTATION_180,PPLAYOUT_ROTATE,
		PP_IMAGE_ROTATION_270,PPLAYOUT_ROTATE,
		PP_IMAGE_SETMASK,PPLAYOUT_MASK,
		PP_IMAGE_SETEFFECTS,PPLAYOUT_EFFECTS,
		PP_IMAGE_SETCOLOURPROFILE,PPLAYOUT_PROFILE,
		PP_LAYOUT_SETBACKGROUND,PPLAYOUT_BACKGROUND,
		0,0
	};
	GtkWidget *w;

	int *p=tags;
	int item=*p++;
	int tag=*p++;
	while(tag)
	{
		w=gtk_item_factory_get_widget_by_action(menu->itemfactory,item);
		if(w)
			gtk_widget_set_sensitive(w,(features&tag)!=0);

		w=gtk_item_factory_get_widget_by_action(menu->popupitemfactory,item);
		if(w)
			gtk_widget_set_sensitive(w,(features&tag)!=0);

		item=*p++;
		tag=*p++;
	}
}


static void pp_menu_set_menu_state_core(GtkItemFactory *itemfactory,bool allowcropping,PP_ROTATION rotation,bool remove)
{
	GtkWidget *w;
	GtkCheckMenuItem *cmi;

	w=gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_REMOVEIMAGE);
	gtk_widget_set_sensitive(w,remove);

	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ALLOWCROPPING));
	gtk_check_menu_item_set_active(cmi,allowcropping);

	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ROTATION_AUTO));
	gtk_check_menu_item_set_active(cmi,rotation==PP_ROTATION_AUTO);
	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ROTATION_NONE));
	gtk_check_menu_item_set_active(cmi,rotation==PP_ROTATION_NONE);
	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ROTATION_90));
	gtk_check_menu_item_set_active(cmi,rotation==PP_ROTATION_90);
	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ROTATION_180));
	gtk_check_menu_item_set_active(cmi,rotation==PP_ROTATION_180);
	cmi=GTK_CHECK_MENU_ITEM(gtk_item_factory_get_widget_by_action(itemfactory,PP_IMAGE_ROTATION_270));
	gtk_check_menu_item_set_active(cmi,rotation==PP_ROTATION_270);
}


void pp_menu_set_menu_state(pp_Menu *menu,bool allowcropping,PP_ROTATION rotation,bool remove)
{
	pp_menu_set_menu_state_core(GTK_ITEM_FACTORY(menu->itemfactory),allowcropping,rotation,remove);
	pp_menu_set_menu_state_core(GTK_ITEM_FACTORY(menu->popupitemfactory),allowcropping,rotation,remove);
	menu->allowcropping=allowcropping;
	menu->rotation=rotation;
}
