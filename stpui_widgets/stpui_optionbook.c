/*
 * stpui_optionbook.c - provides a custom notebook widget for providing autonomous control
 * over an entire stp_vars_t (or at least the Output and Feature classes).
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>

#include "stpui_optionbook.h"
#include "stputil.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_optionbook_signals[LAST_SIGNAL] = { 0 };

static void stpui_optionbook_class_init (stpui_OptionBookClass *klass);
static void stpui_optionbook_init (stpui_OptionBook *stpuicombo);

static void stpui_optionbook_refresh_page(stpui_OptionBook *ob,int page)
{
	gboolean active;
	int index;
	active=stpui_optionpage_refresh(STPUI_OPTIONPAGE(ob->page[page]));

	index=gtk_notebook_page_num(GTK_NOTEBOOK(ob),GTK_WIDGET(ob->scrollwin[page]));
	
	if(active && index<0)
		gtk_notebook_insert_page(GTK_NOTEBOOK(ob),ob->scrollwin[page],ob->label[page],page+ob->custompagecount);

	if(!active && index>-1)
		gtk_notebook_remove_page(GTK_NOTEBOOK(ob),index);
}


void stpui_optionbook_refresh(stpui_OptionBook *ob)
{
	int i;
	for(i=0;i<STPUI_OPTIONBOOK_PAGES;++i)
	{
		stpui_optionbook_refresh_page(ob,i);
	}
}


static void stpui_optionbook_changed(GtkWidget *widget,gpointer *ud)
{
	stpui_OptionBook *ob=STPUI_OPTIONBOOK(ud);

	stputil_validate_parameters(ob->vars);

	stpui_optionbook_refresh(ob);

	g_signal_emit(G_OBJECT (ob),stpui_optionbook_signals[CHANGED_SIGNAL], 0);
}


static void buildpage(stpui_OptionBook *ob,int page,char *name,int optclass,int optlevellow,int optlevelhigh)
{
	ob->scrollwin[page]=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (ob->scrollwin[page]),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	ob->label[page]=gtk_label_new(name);
	gtk_widget_show(ob->scrollwin[page]);
	ob->page[page]=stpui_optionpage_new(ob->vars,optclass,optlevellow,optlevelhigh);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (ob->scrollwin[page]), ob->page[page]);
	gtk_widget_show(ob->page[page]);

	g_signal_connect(G_OBJECT(ob->page[page]),"changed",G_CALLBACK(stpui_optionbook_changed),ob);

	g_object_ref(ob->scrollwin[page]);
	g_object_ref(ob->label[page]);
}


static void init_custompages(stpui_OptionBook *ob)
{
	struct stpui_optionbook_custompage *cps=ob->custompages;
	int page=0;
	
	while(page<ob->custompagecount)
	{
		fprintf(stderr,"Building custom page %d\n",page);
		cps->scrollwin=gtk_scrolled_window_new(NULL,NULL);
	    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (cps->scrollwin),
	                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		cps->label=gtk_label_new(cps->name);
		gtk_widget_show(cps->scrollwin);
		gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (cps->scrollwin), cps->widget);
		gtk_widget_show(cps->widget);

		gtk_notebook_insert_page(GTK_NOTEBOOK(ob),cps->scrollwin,cps->label,page);

		++cps;
		++page;
	}
	fprintf(stderr,"Built custom pages\n");
}


static void populate(stpui_OptionBook *ob)
{
	buildpage(ob,0,"General",STP_PARAMETER_CLASS_FEATURE,
		STP_PARAMETER_LEVEL_BASIC,STP_PARAMETER_LEVEL_BASIC);

	buildpage(ob,1,"Manual Settings",STP_PARAMETER_CLASS_FEATURE,
		STP_PARAMETER_LEVEL_ADVANCED,STP_PARAMETER_LEVEL_ADVANCED4);

	buildpage(ob,2,"Colour (Basic)",STP_PARAMETER_CLASS_OUTPUT,
		STP_PARAMETER_LEVEL_BASIC,STP_PARAMETER_LEVEL_BASIC);

	buildpage(ob,3,"Colour (Fine-tuning)",STP_PARAMETER_CLASS_OUTPUT,
		STP_PARAMETER_LEVEL_ADVANCED,STP_PARAMETER_LEVEL_ADVANCED2);

	buildpage(ob,4,"Colour (Extra)",STP_PARAMETER_CLASS_OUTPUT,
		STP_PARAMETER_LEVEL_ADVANCED3,STP_PARAMETER_LEVEL_ADVANCED4);
}


void stpui_optionbook_rebuild(stpui_OptionBook *ob)
{
	int i;
	int index;
	for(i=0;i<STPUI_OPTIONBOOK_PAGES;++i)
	{
		index=gtk_notebook_page_num(GTK_NOTEBOOK(ob),GTK_WIDGET(ob->scrollwin[i]));
		if(index>=0)
			gtk_notebook_remove_page(GTK_NOTEBOOK(ob),index);
		gtk_widget_destroy(GTK_WIDGET(ob->scrollwin[i]));
	}
	populate(ob);
	stpui_optionbook_refresh(ob);
}


GtkWidget*
stpui_optionbook_new (stp_vars_t *vars,struct stpui_optionbook_custompage *custompages,int custompagecount)
{
	stpui_OptionBook *ob=STPUI_OPTIONBOOK(g_object_new (stpui_optionbook_get_type (), NULL));

	ob->vars=vars;
	ob->custompages=custompages;
	ob->custompagecount=custompagecount;

	init_custompages(ob);

	populate(ob);

	fprintf(stderr,"Pages built - refreshing\n");

	stpui_optionbook_refresh(ob);

	fprintf(stderr,"Done\n");

	return(GTK_WIDGET(ob));
}


GType
stpui_optionbook_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_optionbook_info =
		{
			sizeof (stpui_OptionBookClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_optionbook_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_OptionBook),
			0,
			(GInstanceInitFunc) stpui_optionbook_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_NOTEBOOK, "stpui_OptionBook", &stpui_optionbook_info, 0);
	}
	return stpuic_type;
}


static void
stpui_optionbook_class_init (stpui_OptionBookClass *klass)
{
	stpui_optionbook_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_OptionBookClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_optionbook_init (stpui_OptionBook *ob)
{
	int i;
	for(i=0;i<STPUI_OPTIONBOOK_PAGES;++i)
	{
		ob->page[i]=NULL;
		ob->label[i]=NULL;
		ob->scrollwin[i]=NULL;
	}
}
