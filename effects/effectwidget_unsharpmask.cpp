/*
 * effectwidget_unsharpmask.cpp
 *
 * Copyright (c) 2008 by Alastair M. Robinson
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

#include "effectwidget_unsharpmask.h"

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

static guint effectwidget_unsharpmask_signals[LAST_SIGNAL] = { 0 };

static void effectwidget_unsharpmask_class_init (EffectWidget_UnsharpMaskClass *klass);
static void effectwidget_unsharpmask_init (EffectWidget_UnsharpMask *sel);


static void value_changed(GtkWidget *wid,gpointer obj)
{
	g_signal_emit(G_OBJECT (obj),effectwidget_unsharpmask_signals[CHANGED_SIGNAL], 0);
}

static gchar* format_value_callback (GtkScale *scale,gdouble value,const gpointer userdata)
{
	const gchar *fmt=(const char *)userdata;
	return(g_strdup_printf (fmt,value));
}

GtkWidget*
effectwidget_unsharpmask_new ()
{
	EffectWidget_UnsharpMask *c=EFFECTWIDGET_UNSHARPMASK(g_object_new (effectwidget_unsharpmask_get_type (), NULL));

	c->radiusslider=gtk_hscale_new_with_range(0.1,100,0.1);
	gtk_box_pack_start(GTK_BOX(c),c->radiusslider,TRUE,TRUE,0);
	g_signal_connect(GTK_WIDGET(c->radiusslider),"format-value",G_CALLBACK(format_value_callback),(void*)_("Radius: %1.2f"));
	g_signal_connect(GTK_WIDGET(c->radiusslider),"value-changed",G_CALLBACK(value_changed),c);
	gtk_widget_show(c->radiusslider);

	c->amountslider=gtk_hscale_new_with_range(0.1,5,0.1);
	gtk_box_pack_start(GTK_BOX(c),c->amountslider,TRUE,TRUE,0);
	g_signal_connect(GTK_WIDGET(c->amountslider),"format-value",G_CALLBACK(format_value_callback),(void*)_("Amount: %1.2f"));
	g_signal_connect(GTK_WIDGET(c->amountslider),"value-changed",G_CALLBACK(value_changed),c);
	gtk_widget_show(c->amountslider);

	return(GTK_WIDGET(c));
}


GType
effectwidget_unsharpmask_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo effectwidget_unsharpmask_info =
		{
			sizeof (EffectWidget_UnsharpMaskClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) effectwidget_unsharpmask_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EffectWidget_UnsharpMask),
			0,
			(GInstanceInitFunc) effectwidget_unsharpmask_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "EffectWidget_UnsharpMask", &effectwidget_unsharpmask_info, GTypeFlags(0));
	}
	return stpuic_type;
}


//static void *parent_class=NULL;

static void
effectwidget_unsharpmask_class_init (EffectWidget_UnsharpMaskClass *cls)
{
//	GtkObjectClass *object_class=(GtkObjectClass *)cls;

//	parent_class = gtk_type_class (gtk_widget_get_type ());

	effectwidget_unsharpmask_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectWidget_UnsharpMaskClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void effectwidget_unsharpmask_init (EffectWidget_UnsharpMask *c)
{
	c->radiusslider=NULL;
	c->amountslider=NULL;
}


float effectwidget_unsharpmask_get_radius(EffectWidget_UnsharpMask *tc)
{
	float result=gtk_range_get_value(GTK_RANGE(tc->radiusslider));
	return(result);
}


void effectwidget_unsharpmask_set_radius(EffectWidget_UnsharpMask *tc,float radius)
{
	gtk_range_set_value(GTK_RANGE(tc->radiusslider),radius);
}


float effectwidget_unsharpmask_get_amount(EffectWidget_UnsharpMask *tc)
{
	float result=gtk_range_get_value(GTK_RANGE(tc->amountslider));
	return(result);
}


void effectwidget_unsharpmask_set_amount(EffectWidget_UnsharpMask *tc,float amount)
{
	gtk_range_set_value(GTK_RANGE(tc->amountslider),amount);
}

