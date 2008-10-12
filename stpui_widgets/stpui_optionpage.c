/*
 * stpui_optionpage.c - provides a custom widget for providing autonomous control over
 * a group of options in the stp_vars_t structure.
 *
 * Currently the options to be included are determined by matching a single class
 * and a range of levels.
 *
 * TODO: Allow a callback function to be used to determine the options to be included.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkcheckbutton.h>

#include "stpui_optionpage.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_optionpage_signals[LAST_SIGNAL] = { 0 };

static void stpui_optionpage_class_init (stpui_OptionPageClass *klass);
static void stpui_optionpage_init (stpui_OptionPage *stpuicombo);


gboolean stpui_optionpage_refresh(stpui_OptionPage *op)
{
	gboolean result=FALSE;
	GtkWidget *w;
	int count,i;
	
	count=g_list_length(op->widgetlist);
	for(i=0;i<count;++i)
	{
		GList *listnode=g_list_nth(op->widgetlist,i);
		if(listnode)
		{
			w=GTK_WIDGET(listnode->data);
			
			if(IS_STPUI_COMBO(w))
				result|=stpui_combo_refresh(STPUI_COMBO(w));
			if(IS_STPUI_TOGGLE(w))
				result|=stpui_toggle_refresh(STPUI_TOGGLE(w));
			if(IS_STPUI_SLIDER(w))
				result|=stpui_slider_refresh(STPUI_SLIDER(w));
			if(IS_STPUI_FILE(w))
				result|=stpui_file_refresh(STPUI_FILE(w));
		}
	}
	return(result);
}


static void stpui_optionpage_changed(GtkWidget *widget,gpointer *ud)
{
	stpui_OptionPage *op=STPUI_OPTIONPAGE(ud);

	g_signal_emit(G_OBJECT (op),stpui_optionpage_signals[CHANGED_SIGNAL], 0);
}


static int stpui_optionpage_match(const stp_parameter_t *p,int optclass,int optlevelmin,int optlevelmax)
{
	int result=FALSE;
	if((p->p_class==optclass) || ((optclass<0) && (p->p_class<=STP_PARAMETER_CLASS_OUTPUT)))
	{
		if((p->p_level>=optlevelmin) && (p->p_level<=optlevelmax))
		{
			switch(p->p_type)
			{
				case STP_PARAMETER_TYPE_STRING_LIST:
				case STP_PARAMETER_TYPE_INT:
				case STP_PARAMETER_TYPE_BOOLEAN:
				case STP_PARAMETER_TYPE_DOUBLE:
				case STP_PARAMETER_TYPE_DIMENSION:
				case STP_PARAMETER_TYPE_FILE:
//				case STP_PARAMETER_TYPE_CURVE:
					result=TRUE;
					break;
				default:
					break;
			}
		}
	}
	return(result);
}


static int stpui_optionpage_count(stpui_OptionPage *op)
{
	int result=0;
	int pcount;
	int i;

	stp_parameter_list_t params = stp_get_parameter_list(op->vars);
	pcount = stp_parameter_list_count(params);
	for(i=0;i<pcount;++i)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		if(stpui_optionpage_match(p,op->optclass,op->optlevelmin,op->optlevelmax))
			++result;
	}
	stp_parameter_list_destroy(params);
	return(result);
}


static void stpui_optionpage_build(stpui_OptionPage *op)
{
	int pcount;
	int i;
	int row=0;
	stp_parameter_list_t params;

	params = stp_get_parameter_list(op->vars);
	pcount = stp_parameter_list_count(params);
	for(i=0;i<pcount;++i)
	{
		const stp_parameter_t *p = stp_parameter_list_param(params, i);
		if(stpui_optionpage_match(p,op->optclass,op->optlevelmin,op->optlevelmax))
		{
			GtkWidget *tmp=NULL;
			GtkWidget *label=NULL;
			char *signal="changed";
			if(!p->is_mandatory)
			{
				stp_parameter_t desc;
				stp_describe_parameter(op->vars,p->name,&desc);
				label=gtk_check_button_new_with_label(desc.text);
				stp_parameter_description_destroy(&desc);
			}
			else
			{
				stp_parameter_t desc;
				stp_describe_parameter(op->vars,p->name,&desc);
				label=gtk_label_new(desc.text);
				gtk_misc_set_alignment(GTK_MISC(label),0.0,0.5);
				stp_parameter_description_destroy(&desc);
			}
			switch(p->p_type)
			{
				case STP_PARAMETER_TYPE_STRING_LIST:
					tmp=stpui_combo_new(op->vars,p->name,label);
					signal="changed";
					break;
				case STP_PARAMETER_TYPE_INT:
				case STP_PARAMETER_TYPE_DOUBLE:
					tmp=stpui_slider_new(op->vars,p->name,label);
					signal="changed";
					break;
				case STP_PARAMETER_TYPE_DIMENSION:
					tmp=stpui_slider_new(op->vars,p->name,label);
					signal="changed";
					break;
				case STP_PARAMETER_TYPE_BOOLEAN:
					tmp=stpui_toggle_new(op->vars,p->name,p->text);
					gtk_widget_destroy(label);
					label=NULL;
					signal="toggled";
					break;
				case STP_PARAMETER_TYPE_FILE:
					tmp=stpui_file_new(op->vars,p->name,p->text);
					signal="changed";
					break;
				default:
					gtk_widget_destroy(label);
					label=NULL;
					tmp=NULL;
					break;
			}
			if(tmp)
			{
				gtk_table_attach_defaults(GTK_TABLE(op->table),
					GTK_WIDGET(tmp),
					1,2,
					row,row+1);
				gtk_widget_show(GTK_WIDGET(tmp));
				g_signal_connect(G_OBJECT(tmp),signal,G_CALLBACK(stpui_optionpage_changed),op);
				op->widgetlist=g_list_append(op->widgetlist,tmp);
			}
			if(label)
			{
				gtk_table_attach(GTK_TABLE(op->table),GTK_WIDGET(label),
					0,1,row,row+1,
					GTK_SHRINK|GTK_FILL,GTK_SHRINK,0,0);
				gtk_widget_show(GTK_WIDGET(label));				
				label=FALSE;
			}
			++row;
		}
	}
	stp_parameter_list_destroy(params);
}


GtkWidget*
stpui_optionpage_new (stp_vars_t *vars,int optclass,int optlevelmin,int optlevelmax)
{
	int count=0;
	stpui_OptionPage *op=STPUI_OPTIONPAGE(g_object_new (stpui_optionpage_get_type (), NULL));

	op->vars=vars;
	op->optclass=optclass;
	op->optlevelmin=optlevelmin;
	op->optlevelmax=optlevelmax;

	count=stpui_optionpage_count(op);
	op->table=gtk_table_new(count,2,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(op->table),5);
	
	gtk_table_set_col_spacing(GTK_TABLE(op->table),0,8);
	gtk_box_pack_start(GTK_BOX(op),GTK_WIDGET(op->table),FALSE,TRUE,0);
	gtk_widget_show(GTK_WIDGET(op->table));

	stpui_optionpage_build(op);

	return(GTK_WIDGET(op));
}


GType
stpui_optionpage_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_optionpage_info =
		{
			sizeof (stpui_OptionPageClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_optionpage_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_OptionPage),
			0,
			(GInstanceInitFunc) stpui_optionpage_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "stpui_OptionPage", &stpui_optionpage_info, 0);
	}
	return stpuic_type;
}


static void
stpui_optionpage_class_init (stpui_OptionPageClass *klass)
{
	stpui_optionpage_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_OptionPageClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_optionpage_init (stpui_OptionPage *op)
{
	op->widgetlist=NULL;
}
