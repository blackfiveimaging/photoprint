/*
 * stpui_toggle.c - provides a custom widget for providing autonomous control over
 * a boolean option in the stp_vars_t structure.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <string.h>

#include "stpui_toggle.h"

enum {
	TOGGLED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_toggle_signals[LAST_SIGNAL] = { 0 };

static void stpui_toggle_class_init (stpui_ToggleClass *klass);
static void stpui_toggle_init (stpui_Toggle *t);


static gboolean stpui_toggle_set_state(stpui_Toggle *c)
{
	stp_parameter_t desc;
	gboolean result;

	stp_describe_parameter(c->vars,c->optionname,&desc);
	if((desc.p_type==STP_PARAMETER_TYPE_BOOLEAN) && desc.is_active)
	{
		int active=stp_get_boolean_parameter(c->vars,c->optionname);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->toggle),active);
	}
	if(desc.is_active)
		gtk_widget_show(GTK_WIDGET(c));
	else
		gtk_widget_hide(GTK_WIDGET(c));
//	gtk_widget_set_sensitive(GTK_WIDGET(c->toggle),desc.is_active);
	result=desc.is_active;
	stp_parameter_description_destroy(&desc);
	return(result);
}


static void stpui_toggle_changed(GtkToggleButton *tb,gpointer *ud)
{
	stpui_Toggle *t=STPUI_TOGGLE(ud);
	gboolean active=gtk_toggle_button_get_active(tb);
	stp_set_boolean_parameter(t->vars,t->optionname,active);
	g_signal_emit(G_OBJECT (t),
		stpui_toggle_signals[TOGGLED_SIGNAL], 0);
}


GtkWidget*
stpui_toggle_new (stp_vars_t *vars,const char *optname,const char *displayname)
{
	stpui_Toggle *c=STPUI_TOGGLE(g_object_new (stpui_toggle_get_type (), NULL));

	c->toggle=gtk_toggle_button_new_with_label(displayname);

	c->vars=vars;
	c->optionname=optname;
	
	stpui_toggle_set_state(c);
	g_signal_connect(GTK_WIDGET(c->toggle),"toggled",G_CALLBACK(stpui_toggle_changed),c);

	gtk_box_pack_end(GTK_BOX(c),GTK_WIDGET(c->toggle),TRUE,TRUE,0);
	gtk_widget_show(c->toggle);
	
	return(GTK_WIDGET(c));
}


GType
stpui_toggle_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_toggle_info =
		{
			sizeof (stpui_ToggleClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_toggle_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_Toggle),
			0,
			(GInstanceInitFunc) stpui_toggle_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "stpui_Toggle", &stpui_toggle_info, 0);
	}
	return stpuic_type;
}


static void
stpui_toggle_class_init (stpui_ToggleClass *klass)
{
	stpui_toggle_signals[TOGGLED_SIGNAL] =
	g_signal_new ("toggled",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_ToggleClass, toggled),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_toggle_init (stpui_Toggle *c)
{
}


gboolean stpui_toggle_refresh(stpui_Toggle *t)
{
	gboolean result;

	g_signal_handlers_block_matched (G_OBJECT (t->toggle), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, t);

	result=stpui_toggle_set_state(t);

	g_signal_handlers_unblock_matched (G_OBJECT (t->toggle), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, t);
	return(result);
}
