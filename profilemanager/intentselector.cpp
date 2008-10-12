/*
 * intentselector.c - provides a custom widget for choosing a rendering intent
 *
 * Copyright (c) 2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtktooltips.h>

#include "intentselector.h"

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint intentselector_signals[LAST_SIGNAL] = { 0 };

static void intentselector_class_init (IntentSelectorClass *klass);
static void intentselector_init (IntentSelector *stpuicombo);


static void intentselector_build_options(IntentSelector *c)
{
	if(c->menu)
		gtk_option_menu_remove_menu(GTK_OPTION_MENU(c->optionmenu));
	c->menu=gtk_menu_new();

	int count=c->pm->GetIntentCount();
	for(int i=-1;i<count;++i)
	{
		const char *name=c->pm->GetIntentName(LCMSWrapper_Intent(i));
		if(name)
		{
			GtkWidget *menu_item = gtk_menu_item_new_with_label (name);
			gtk_menu_shell_append (GTK_MENU_SHELL (c->menu), menu_item);
			gtk_widget_show (menu_item);
		}
	}
	gtk_option_menu_set_menu(GTK_OPTION_MENU(c->optionmenu),c->menu);
}


static void	intentselector_entry_changed(GtkEntry *entry,gpointer user_data)
{
	IntentSelector *c=INTENTSELECTOR(user_data);

	int index=intentselector_getintent(c);
	const char *desc=c->pm->GetIntentDescription(LCMSWrapper_Intent(index));
	if(desc && strlen(desc)>0)
	{
		gtk_tooltips_set_tip(c->tips,c->optionmenu,desc,desc);
		gtk_tooltips_enable(c->tips);
	}
	else
		gtk_tooltips_disable(c->tips);
	
	g_signal_emit(G_OBJECT (c),intentselector_signals[CHANGED_SIGNAL], 0);
}


GtkWidget*
intentselector_new (ProfileManager *pm)
{
	IntentSelector *c=INTENTSELECTOR(g_object_new (intentselector_get_type (), NULL));

	c->pm=pm;

	c->tips=gtk_tooltips_new();
	c->optionmenu=gtk_option_menu_new();
	c->menu=NULL;  // Built on demand...

	intentselector_build_options(c);

	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->optionmenu),TRUE,TRUE,0);
	gtk_widget_show(c->optionmenu);

	g_signal_connect(c->optionmenu,"changed",G_CALLBACK(intentselector_entry_changed),c);
	
	return(GTK_WIDGET(c));
}


GType
intentselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo intentselector_info =
		{
			sizeof (IntentSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) intentselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (IntentSelector),
			0,
			(GInstanceInitFunc) intentselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "IntentSelector", &intentselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
intentselector_class_init (IntentSelectorClass *klass)
{
	intentselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (IntentSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
intentselector_init (IntentSelector *c)
{
	c->optionmenu=NULL;
	c->menu=NULL;
	c->pm=NULL;
}


LCMSWrapper_Intent intentselector_getintent(IntentSelector *c)
{
	gint index=gtk_option_menu_get_history(GTK_OPTION_MENU(c->optionmenu));
	return(LCMSWrapper_Intent(index-1));
}


void intentselector_setintent(IntentSelector *c,LCMSWrapper_Intent intent)
{
	int index=intent;
	gtk_option_menu_set_history(GTK_OPTION_MENU(c->optionmenu),index+1);
	const char *desc=c->pm->GetIntentDescription(intent);
	if(desc)
	{
		gtk_tooltips_set_tip(c->tips,c->optionmenu,desc,desc);
		gtk_tooltips_enable(c->tips);
	}
	else
		gtk_tooltips_disable(c->tips);
}
