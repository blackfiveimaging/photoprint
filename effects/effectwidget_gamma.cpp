/*
 * effectwidget_gamma.c - provides a list of available effects.
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

#include "../support/debug.h"

#include "ppeffect.h"
#include "ppeffect_desaturate.h"
#include "ppeffect_temperature.h"

#include "effectlist.h"

#include "effectwidget_gamma.h"

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

static guint effectwidget_gamma_signals[LAST_SIGNAL] = { 0 };

static void effectwidget_gamma_class_init (EffectWidget_GammaClass *klass);
static void effectwidget_gamma_init (EffectWidget_Gamma *sel);


static void value_changed(GtkWidget *wid,gpointer obj)
{
	Debug[TRACE] << "Tempchange got changed signal from slider - emitting signal" << endl;
	g_signal_emit(G_OBJECT (obj),effectwidget_gamma_signals[CHANGED_SIGNAL], 0);
}

static gchar* format_value_callback (GtkScale *scale,gdouble value,gpointer userdata)
{
	return(g_strdup_printf (_("Gamma: %f"),value));
}

GtkWidget*
effectwidget_gamma_new ()
{
	EffectWidget_Gamma *c=EFFECTWIDGET_GAMMA(g_object_new (effectwidget_gamma_get_type (), NULL));

	c->slider=gtk_hscale_new_with_range(0.4,2.5,0.05);
	gtk_box_pack_start(GTK_BOX(c),c->slider,TRUE,TRUE,0);
	g_signal_connect(GTK_WIDGET(c->slider),"format-value",G_CALLBACK(format_value_callback),c);
	g_signal_connect(GTK_WIDGET(c->slider),"value-changed",G_CALLBACK(value_changed),c);

	gtk_widget_show(c->slider);

	return(GTK_WIDGET(c));
}


GType
effectwidget_gamma_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo effectwidget_gamma_info =
		{
			sizeof (EffectWidget_GammaClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) effectwidget_gamma_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EffectWidget_Gamma),
			0,
			(GInstanceInitFunc) effectwidget_gamma_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "EffectWidget_Gamma", &effectwidget_gamma_info, GTypeFlags(0));
	}
	return stpuic_type;
}


//static void *parent_class=NULL;

static void
effectwidget_gamma_class_init (EffectWidget_GammaClass *cls)
{
//	GtkObjectClass *object_class=(GtkObjectClass *)cls;

//	parent_class = gtk_type_class (gtk_widget_get_type ());

	effectwidget_gamma_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectWidget_GammaClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void effectwidget_gamma_init (EffectWidget_Gamma *c)
{
	c->slider=NULL;
}


double effectwidget_gamma_get(EffectWidget_Gamma *tc)
{
	double result=gtk_range_get_value(GTK_RANGE(tc->slider));
	return(result);
}


void effectwidget_gamma_set(EffectWidget_Gamma *tc,double gamma)
{
	gtk_range_set_value(GTK_RANGE(tc->slider),gamma);
}
