/*
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtk.h>
#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenuitem.h>

#include "config.h"

#include "pp_units.h"
#include "dialogs.h"

#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_units_signals[LAST_SIGNAL] = { 0 };

static void pp_units_class_init (pp_UnitsClass *klass);
static void pp_units_init (pp_Units *stpuicombo);


void pp_units_refresh(pp_Units *ob)
{

}


GtkWidget*
pp_units_new ()
{
	pp_Units *ob=PP_UNITS(g_object_new (pp_units_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	GtkWidget *label;
	GtkWidget *hbox;

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(ob),hbox,TRUE,TRUE,0);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Units:"));
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_widget_show(label);

	ob->unitselector = gtk_option_menu_new ();      
	GtkWidget *menu, *menu_item;
	menu = gtk_menu_new ();

	menu_item = gtk_menu_item_new_with_label (_("Points"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label (_("Inches"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label (_("Millimeters"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	menu_item = gtk_menu_item_new_with_label (_("Centimeters"));
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);
	
	gtk_option_menu_set_menu (GTK_OPTION_MENU (ob->unitselector), menu);
	
	gtk_box_pack_start(GTK_BOX(hbox),ob->unitselector,TRUE,TRUE,5);
	gtk_widget_show(ob->unitselector);

	pp_units_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_units_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_units_info =
		{
			sizeof (pp_UnitsClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_units_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Units),
			0,
			(GInstanceInitFunc) pp_units_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "pp_Units", &pp_units_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_units_class_init (pp_UnitsClass *klass)
{
	pp_units_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_UnitsClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_units_init (pp_Units *ob)
{
}


void pp_units_set_unit(pp_Units *ob,enum Units unit)
{
	gtk_option_menu_set_history(GTK_OPTION_MENU(ob->unitselector),unit);
}


enum Units pp_units_get_unit(pp_Units *ob)
{
	int u=gtk_option_menu_get_history(GTK_OPTION_MENU(ob->unitselector));
	return(Units(u));
}


enum Units pp_units_run_dialog(enum Units unit,GtkWindow *parent)
{
	GtkWidget *dialog=gtk_dialog_new_with_buttons(_("Units"),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	GtkWidget *uw=pp_units_new();
	pp_units_set_unit(PP_UNITS(uw),unit);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),uw,FALSE,FALSE,0);
	gtk_widget_show(uw);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			unit=pp_units_get_unit(PP_UNITS(uw));
			break;
	}
	gtk_widget_destroy(dialog);
	return(unit);
}
