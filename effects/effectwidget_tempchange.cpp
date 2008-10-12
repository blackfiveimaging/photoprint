/*
 * effectwidget_tempchange.c - provides a list of available effects.
 *
 * Copyright (c) 2007 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <iostream>

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "ppeffect.h"
#include "ppeffect_desaturate.h"
#include "ppeffect_temperature.h"

#include "effectlist.h"

#include "effectwidget_tempchange.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint effectwidget_tempchange_signals[LAST_SIGNAL] = { 0 };

static void effectwidget_tempchange_class_init (EffectWidget_TempChangeClass *klass);
static void effectwidget_tempchange_init (EffectWidget_TempChange *sel);


static void value_changed(GtkWidget *wid,gpointer obj)
{
	cerr << "Got changed signal from slider - emitting signal" << endl;
	g_signal_emit(G_OBJECT (obj),effectwidget_tempchange_signals[CHANGED_SIGNAL], 0);
}

static gchar* format_value_callback (GtkScale *scale,gdouble value,gpointer userdata)
{
	int t=int(value);
	return(g_strdup_printf (_("%d degrees K"),t*50));
}

GtkWidget*
effectwidget_tempchange_new ()
{
	EffectWidget_TempChange *c=EFFECTWIDGET_TEMPCHANGE(g_object_new (effectwidget_tempchange_get_type (), NULL));

	c->slider=gtk_hscale_new_with_range(-40,40,1);
	gtk_box_pack_start(GTK_BOX(c),c->slider,TRUE,TRUE,0);
	g_signal_connect(GTK_WIDGET(c->slider),"format-value",G_CALLBACK(format_value_callback),c);
	g_signal_connect(GTK_WIDGET(c->slider),"value-changed",G_CALLBACK(value_changed),c);

	gtk_widget_show(c->slider);

	return(GTK_WIDGET(c));
}


GType
effectwidget_tempchange_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo effectwidget_tempchange_info =
		{
			sizeof (EffectWidget_TempChangeClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) effectwidget_tempchange_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EffectWidget_TempChange),
			0,
			(GInstanceInitFunc) effectwidget_tempchange_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "EffectWidget_TempChange", &effectwidget_tempchange_info, GTypeFlags(0));
	}
	return stpuic_type;
}


//static void *parent_class=NULL;

static void
effectwidget_tempchange_class_init (EffectWidget_TempChangeClass *cls)
{
//	GtkObjectClass *object_class=(GtkObjectClass *)cls;

//	parent_class = gtk_type_class (gtk_widget_get_type ());

	effectwidget_tempchange_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectWidget_TempChangeClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void effectwidget_tempchange_init (EffectWidget_TempChange *c)
{
	c->slider=NULL;
}


int effectwidget_tempchange_get(EffectWidget_TempChange *tc)
{
	int result=int(gtk_range_get_value(GTK_RANGE(tc->slider)));
	return(result*50);
}


void effectwidget_tempchange_set(EffectWidget_TempChange *tc,int temp)
{
	gtk_range_set_value(GTK_RANGE(tc->slider),temp/50);
}
