/*
 * stpui_combo.c - provides a custom widget for providing autonomous control over
 * a string option in the stp_vars_t structure.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


/* FIXME:  A destructor is needed for this class, to free up the option list.
   As it stands, a certain amount of memory will be lost when the widget is
   destroyed.
*/

#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>

#include "stpui_combo.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_combo_signals[LAST_SIGNAL] = { 0 };

static void stpui_combo_class_init (stpui_ComboClass *klass);
static void stpui_combo_init (stpui_Combo *stpuicombo);


static char *stpui_combo_get_optionkey(stpui_Combo *c,const char *longname,stp_parameter_t *desc)
{
	char *result=NULL;
	stp_parameter_t d;
	int free_desc=FALSE;

	if(!desc)
	{
		stp_describe_parameter(c->vars,c->optionname,&d);
		free_desc=TRUE;
		desc=&d;
	}

	if(desc->p_type==STP_PARAMETER_TYPE_STRING_LIST)
	{
		stp_string_list_t *strlist=desc->bounds.str;
		if(strlist)
		{
			int strcount;
			int j;
			strcount=stp_string_list_count(strlist);
			for(j=0;j<strcount;++j)
			{
				stp_param_string_t *p=stp_string_list_param(strlist,j);
				gsize in,out;
				char *utftext=g_locale_to_utf8(p->text,-1,&in,&out,NULL);
				if(strcmp(utftext,longname)==0)
				{
					result=g_strdup(p->name);
				}
				g_free(utftext);
			}
		}
	}

	if(free_desc)
		stp_parameter_description_destroy(&d);

	return(result);
}


static char *stpui_combo_get_longname(stpui_Combo *c,const char *shortname,stp_parameter_t *desc)
{
	char *result=NULL;
	stp_parameter_t d;
	int free_desc=FALSE;

	if(!desc)
	{
		stp_describe_parameter(c->vars,c->optionname,&d);
		free_desc=TRUE;
		desc=&d;
	}

	if(desc->p_type==STP_PARAMETER_TYPE_STRING_LIST)
	{
		stp_string_list_t *strlist=desc->bounds.str;
		if(strlist)
		{
			int strcount;
			int j;
			strcount=stp_string_list_count(strlist);
			for(j=0;j<strcount;++j)
			{
				stp_param_string_t *p=stp_string_list_param(strlist,j);
				if(strcmp(p->name,shortname)==0)
				{
					gsize in,out;
					result=g_locale_to_utf8(p->text,-1,&in,&out,NULL);
				}
			}
		}
	}

	if(free_desc)
		stp_parameter_description_destroy(&d);

	return(result);
}


static gboolean stpui_combo_build_options(stpui_Combo *c)
{
	stp_parameter_t desc;
	const char *value;
	char *longname;
	gboolean result;

	if(c->optionlist)
	{
		GList *element;
		gtk_list_clear_items(GTK_LIST(GTK_COMBO(c->combo)->list),0,-1);
		element=c->optionlist;
		while(element)
		{
			g_free(element->data);
			element=g_list_next(element);
		}
		g_list_free(c->optionlist);
		c->optionlist=NULL;
	}

	stp_describe_parameter(c->vars,c->optionname,&desc);
	if((desc.p_type==STP_PARAMETER_TYPE_STRING_LIST) && desc.is_active)
	{
		int strcount,j;
		stp_string_list_t *strlist=desc.bounds.str;
		if(strlist)
		{
			strcount=stp_string_list_count(strlist);
			for(j=0;j<strcount;++j)
			{
				gsize in,out;
				stp_param_string_t *p=stp_string_list_param(strlist,j);
				c->optionlist=g_list_append(c->optionlist,g_locale_to_utf8(p->text,-1,&in,&out,NULL));
			}
			gtk_combo_set_popdown_strings (GTK_COMBO (c->combo), c->optionlist);
		}
		value=stp_get_string_parameter(c->vars,c->optionname);
		if(!value)
			value=desc.deflt.str;
		longname=stpui_combo_get_longname(c,value,&desc);
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(c->combo)->entry),longname);
		g_free(longname);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(c->combo),desc.is_active && c->optionlist);
	result=desc.is_active;
	stp_parameter_description_destroy(&desc);

	return(result);
}


static void	stpui_combo_entry_changed(GtkEntry *entry,gpointer user_data)
{
	stpui_Combo *combo=STPUI_COMBO(user_data);
	GtkCombo *c=GTK_COMBO(combo->combo);

	fprintf(stderr,"Received entry changed for combo\n");

	const char *val=gtk_entry_get_text(GTK_ENTRY(c->entry));

	fprintf(stderr,"Current value: %s\n",val);

	if(val && strlen(val))
	{
		char *optkey;
		optkey=stpui_combo_get_optionkey(combo,val,NULL);
		if(optkey)
		{
			fprintf(stderr,"Option key: %s\n",optkey);
			stp_set_string_parameter(combo->vars,combo->optionname,optkey);
			g_signal_emit(G_OBJECT (combo),
				stpui_combo_signals[CHANGED_SIGNAL], 0);
		}
		else
			fprintf(stderr,"Can't find optionkey\n");
	}
}


static void stpui_toggle_changed(GtkWidget *w,gpointer *ud)
{
	stpui_Combo *s=STPUI_COMBO(ud);

	fprintf(stderr,"Received toggle changed for combo\n");

	gboolean enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(s->checkbutton));
	if(enabled)
	{
		const char *val=gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(s->combo)->entry));
		if(val && strlen(val))
		{
			char *optkey;
			optkey=stpui_combo_get_optionkey(s,val,NULL);
			if(optkey)
			{
				stp_set_string_parameter(s->vars,s->optionname,optkey);
			}
		}
	}
	else
	{
		stp_clear_string_parameter(s->vars,s->optionname);
	}
	
	stpui_combo_refresh(s);
}


GtkWidget*
stpui_combo_new (stp_vars_t *vars,const char *optname,GtkWidget *label)
{
	stpui_Combo *c=STPUI_COMBO(g_object_new (stpui_combo_get_type (), NULL));

	c->combo=gtk_combo_new();
	gtk_combo_set_value_in_list(GTK_COMBO(c->combo),TRUE,FALSE);

	c->vars=vars;
	c->optionname=optname;

	if(GTK_IS_CHECK_BUTTON(label))
		c->checkbutton=GTK_CHECK_BUTTON(label);
	else
		c->label=label;

	if(c->checkbutton)
	{
		gboolean active=stp_check_string_parameter(c->vars,c->optionname,STP_PARAMETER_DEFAULTED);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(c->checkbutton),active);
		g_signal_connect(G_OBJECT(c->checkbutton),"toggled",G_CALLBACK(stpui_toggle_changed),c);
	}

	stpui_combo_build_options(c);
	g_signal_connect(GTK_COMBO(c->combo)->entry,"changed",G_CALLBACK(stpui_combo_entry_changed),c);

	gtk_box_pack_end(GTK_BOX(c),GTK_WIDGET(c->combo),TRUE,TRUE,0);
	gtk_widget_show(c->combo);
	
	return(GTK_WIDGET(c));
}


GType
stpui_combo_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_combo_info =
		{
			sizeof (stpui_ComboClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_combo_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_Combo),
			0,
			(GInstanceInitFunc) stpui_combo_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "stpui_Combo", &stpui_combo_info, 0);
	}
	return stpuic_type;
}


static void
stpui_combo_class_init (stpui_ComboClass *klass)
{
	stpui_combo_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_ComboClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_combo_init (stpui_Combo *c)
{
	c->optionlist=NULL;
	c->checkbutton=NULL;
	c->label=NULL;
}


gboolean stpui_combo_refresh(stpui_Combo *c)
{
	gboolean result;
	gboolean enabled=TRUE;

	if(c->checkbutton)
		enabled=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(c->checkbutton));

	gtk_widget_set_sensitive(GTK_WIDGET(c),enabled);

	g_signal_handlers_block_matched (G_OBJECT (GTK_COMBO(c->combo)->list), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);
	g_signal_handlers_block_matched (G_OBJECT (GTK_COMBO(c->combo)->entry), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);

	if((result=stpui_combo_build_options(c)))
	{
		gtk_widget_show(GTK_WIDGET(c));
		if(c->checkbutton)
			gtk_widget_show(GTK_WIDGET(c->checkbutton));
		if(c->label)
			gtk_widget_show(c->label);
	}
	else
	{
		gtk_widget_hide(GTK_WIDGET(c));
		if(c->checkbutton)
			gtk_widget_hide(GTK_WIDGET(c->checkbutton));
		if(c->label)
			gtk_widget_hide(c->label);
	}
	g_signal_handlers_unblock_matched (G_OBJECT (GTK_COMBO(c->combo)->list), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);
	g_signal_handlers_unblock_matched (G_OBJECT (GTK_COMBO(c->combo)->entry), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, c);
	return(result);
}
