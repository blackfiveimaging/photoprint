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

#include "pp_scaling.h"
#include "dialogs.h"

#include "gettext.h"
#define _(x) gettext(x)

#include "imagesource/imagesource_util.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_scaling_signals[LAST_SIGNAL] = { 0 };

static void pp_scaling_class_init (pp_ScalingClass *klass);
static void pp_scaling_init (pp_Scaling *stpuicombo);


static void scaling_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Scaling *lo=(pp_Scaling *)ob;

	pp_scaling_refresh(lo);
	
	g_signal_emit(G_OBJECT (ob),pp_scaling_signals[CHANGED_SIGNAL], 0);
}


void pp_scaling_refresh(pp_Scaling *ob)
{
	pp_scaling_get_scale(ob);
}


GtkWidget*
pp_scaling_new (IS_ScalingQuality scale)
{
	pp_Scaling *ob=PP_SCALING(g_object_new (pp_scaling_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	GtkWidget *label;
	GtkWidget *hbox;

	hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(ob),hbox,TRUE,TRUE,0);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Algorithm:"));
	gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
	gtk_widget_show(label);

	SimpleComboOptions opts;
	for(int i=0;i<IS_SCALING_MAX;++i)
	{
		const IS_ScalingQualityDescription *desc=DescribeScalingQuality(IS_ScalingQuality(i));
		opts.Add(desc->Name,gettext(desc->Name),gettext(desc->Description));
	}

	ob->scaleselector=simplecombo_new(opts);

#if 0

	ob->scaleselector = gtk_option_menu_new ();      
	GtkWidget *menu, *menu_item;
	menu = gtk_menu_new ();

	for(int i=0;i<IS_SCALING_MAX;++i)
	{
		const IS_ScalingQualityDescription *desc=DescribeScalingQuality(IS_ScalingQuality(i));
		menu_item = gtk_menu_item_new_with_label (gettext(desc->Name));
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
		g_signal_connect(G_OBJECT(menu_item),"activate",G_CALLBACK(scaling_changed),ob);
		gtk_widget_show (menu_item);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (ob->scaleselector), menu);
#endif	

	g_signal_connect(G_OBJECT(ob->scaleselector),"changed",G_CALLBACK(scaling_changed),ob);
	gtk_box_pack_start(GTK_BOX(hbox),ob->scaleselector,TRUE,TRUE,5);
	gtk_widget_show(ob->scaleselector);

	ob->description=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(ob),ob->description,TRUE,TRUE,0);
	gtk_widget_show(ob->description);

	pp_scaling_set_scale(ob,scale);

	pp_scaling_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_scaling_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_scaling_info =
		{
			sizeof (pp_ScalingClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_scaling_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Scaling),
			0,
			(GInstanceInitFunc) pp_scaling_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "pp_Scaling", &pp_scaling_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_scaling_class_init (pp_ScalingClass *klass)
{
	pp_scaling_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_ScalingClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_scaling_init (pp_Scaling *ob)
{
}


void pp_scaling_set_scale(pp_Scaling *ob,IS_ScalingQuality scale)
{
//	gtk_option_menu_set_history(GTK_OPTION_MENU(ob->scaleselector),scale);
	simplecombo_set_index(SIMPLECOMBO(ob->scaleselector),scale);
	const IS_ScalingQualityDescription *desc=DescribeScalingQuality(IS_ScalingQuality(scale));
	gtk_label_set_text(GTK_LABEL(ob->description),gettext(desc->Description));
}


IS_ScalingQuality pp_scaling_get_scale(pp_Scaling *ob)
{
//	IS_ScalingQuality s=IS_ScalingQuality(gtk_option_menu_get_history(GTK_OPTION_MENU(ob->scaleselector)));
	IS_ScalingQuality s=IS_ScalingQuality(simplecombo_get_index(SIMPLECOMBO(ob->scaleselector)));
	const IS_ScalingQualityDescription *desc=DescribeScalingQuality(IS_ScalingQuality(s));
	gtk_label_set_text(GTK_LABEL(ob->description),gettext(desc->Description));
	return(s);
}


IS_ScalingQuality pp_scaling_run_dialog(GtkWindow *parent,IS_ScalingQuality scaling)
{
	GtkWidget *dialog=gtk_dialog_new_with_buttons(_("Scaling"),
		parent,GtkDialogFlags(0),
		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	GtkWidget *uw=pp_scaling_new(scaling);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),uw,FALSE,FALSE,0);
	gtk_widget_show(uw);

	gtk_widget_show(dialog);
	gint result=gtk_dialog_run(GTK_DIALOG(dialog));
	switch(result)
	{
		case GTK_RESPONSE_CANCEL:
			break;
		case GTK_RESPONSE_OK:
			scaling=pp_scaling_get_scale(PP_SCALING(uw));
			break;
	}
	gtk_widget_destroy(dialog);
	return(scaling);
}
