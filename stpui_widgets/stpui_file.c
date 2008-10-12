/*
 * stpui_file.c - provides a custom widget for providing autonomous control over
 * a boolean option in the stp_vars_t structure.
 *
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <string.h>
#include <gtk/gtk.h>
#include "stpui_file.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_file_signals[LAST_SIGNAL] = { 0 };

static void stpui_file_class_init (stpui_FileClass *klass);
static void stpui_file_init (stpui_File *t);


static gboolean stpui_file_set_state(stpui_File *c)
{
	stp_parameter_t desc;
	gboolean result;

	stp_describe_parameter(c->vars,c->optionname,&desc);
	if((desc.p_type==STP_PARAMETER_TYPE_FILE) && desc.is_active)
	{
		const char *file=stp_get_file_parameter(c->vars,c->optionname);
		if(!file)
			file="";
		gtk_entry_set_text(GTK_ENTRY(c->entry),file);
	}
	gtk_widget_set_sensitive(GTK_WIDGET(c->entry),desc.is_active);
	gtk_widget_set_sensitive(GTK_WIDGET(c->button),desc.is_active);
	fprintf(stderr,"File widget is active?: %d\n",desc.is_active);
	result=desc.is_active;
	stp_parameter_description_destroy(&desc);
	return(result);
}


static void stpui_file_changed(GtkWidget *w,gpointer *ud)
{
	stpui_File *t=STPUI_FILE(ud);

	const char *text=gtk_entry_get_text(GTK_ENTRY(t->entry));
	stp_set_file_parameter(t->vars,t->optionname,text);

	g_signal_emit(G_OBJECT (t),
		stpui_file_signals[CHANGED_SIGNAL], 0);
}


static void getfile(GtkWidget *w,gpointer *ud)
{
	stpui_File *t=STPUI_FILE(ud);

	const char *oldfilename=gtk_entry_get_text(GTK_ENTRY(t->entry));

	GtkWidget *dialog;

	dialog = gtk_file_chooser_dialog_new ("Choose PPD File...",
		GTK_WINDOW(NULL),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	if(oldfilename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),oldfilename);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		gtk_entry_set_text(GTK_ENTRY(t->entry),gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)));

	gtk_widget_destroy(GTK_WIDGET(dialog));
}


GtkWidget*
stpui_file_new (stp_vars_t *vars,const char *optname,const char *displayname)
{
	stpui_File *c=STPUI_FILE(g_object_new (stpui_file_get_type (), NULL));

	c->vars=vars;
	c->optionname=optname;

	c->entry=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->entry),TRUE,TRUE,0);
	gtk_widget_show(c->entry);

	c->button=gtk_button_new_with_label("...");
	gtk_box_pack_start(GTK_BOX(c),GTK_WIDGET(c->button),FALSE,FALSE,0);

	stpui_file_set_state(c);

	g_signal_connect(GTK_WIDGET(c->entry),"changed",G_CALLBACK(stpui_file_changed),c);
	g_signal_connect(GTK_WIDGET(c->button),"clicked",G_CALLBACK(getfile),c);

	gtk_widget_show(c->button);

	return(GTK_WIDGET(c));
}


GType
stpui_file_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_file_info =
		{
			sizeof (stpui_FileClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_file_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_File),
			0,
			(GInstanceInitFunc) stpui_file_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "stpui_File", &stpui_file_info, 0);
	}
	return stpuic_type;
}


static void
stpui_file_class_init (stpui_FileClass *klass)
{
	stpui_file_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_FileClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_file_init (stpui_File *c)
{
}


gboolean stpui_file_refresh(stpui_File *t)
{
	gboolean result;

	g_signal_handlers_block_matched (G_OBJECT (t->entry), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, t);

	result=stpui_file_set_state(t);

	g_signal_handlers_unblock_matched (G_OBJECT (t->entry), 
                                         G_SIGNAL_MATCH_DATA,
                                         0, 0, NULL, NULL, t);
	return(result);
}
