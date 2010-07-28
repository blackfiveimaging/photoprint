/*
 * effectwidget_channelmask.c - provides a list of available effects.
 *
 * Copyright (c) 2010 by Alastair M. Robinson
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

#include "effectwidget_channelmask.h"

#include "../miscwidgets/coloranttoggle.h"

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

static guint effectwidget_channelmask_signals[LAST_SIGNAL] = { 0 };

static void effectwidget_channelmask_class_init (EffectWidget_ChannelMaskClass *klass);
static void effectwidget_channelmask_init (EffectWidget_ChannelMask *sel);


static void changed(GtkWidget *wid,gpointer obj)
{
	Debug[TRACE] << "ColorantToggle got changed signal" << endl;
	g_signal_emit(G_OBJECT (obj),effectwidget_channelmask_signals[CHANGED_SIGNAL], 0);
}


GtkWidget*
effectwidget_channelmask_new (DeviceNColorantList *list)
{
	EffectWidget_ChannelMask *c=EFFECTWIDGET_CHANNELMASK(g_object_new (effectwidget_channelmask_get_type (), NULL));

	c->channels=coloranttoggle_new(list);
	gtk_box_pack_start(GTK_BOX(c),c->channels,TRUE,TRUE,0);
	g_signal_connect(GTK_WIDGET(c->channels),"changed",G_CALLBACK(changed),c);

	gtk_widget_show(c->channels);

	return(GTK_WIDGET(c));
}


GType
effectwidget_channelmask_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo effectwidget_channelmask_info =
		{
			sizeof (EffectWidget_ChannelMaskClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) effectwidget_channelmask_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EffectWidget_ChannelMask),
			0,
			(GInstanceInitFunc) effectwidget_channelmask_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "EffectWidget_ChannelMask", &effectwidget_channelmask_info, GTypeFlags(0));
	}
	return stpuic_type;
}


//static void *parent_class=NULL;

static void
effectwidget_channelmask_class_init (EffectWidget_ChannelMaskClass *cls)
{
//	GtkObjectClass *object_class=(GtkObjectClass *)cls;

//	parent_class = gtk_type_class (gtk_widget_get_type ());

	effectwidget_channelmask_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectWidget_ChannelMaskClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void effectwidget_channelmask_init (EffectWidget_ChannelMask *c)
{
	c->channels=NULL;
}


//double effectwidget_channelmask_get(EffectWidget_ChannelMask *tc)
//{
//	double result=gtk_range_get_value(GTK_RANGE(tc->slider));
//	return(result);
//}


void effectwidget_channelmask_set(EffectWidget_ChannelMask *tc,DeviceNColorantList *list)
{
	coloranttoggle_set_colorants(COLORANTTOGGLE(tc->channels),list);
}
