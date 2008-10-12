/*
 * patheditor.c - provides a custom widget for manipulating a SearchPathHandler object.
 *
 * Copyright (c) 2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


/* FIXME:  A destructor is needed for this class, to free up the option list.
   As it stands, a certain amount of memory will be lost when the widget is
   destroyed.
*/
#include <iostream>

#include <string.h>

#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkscrolledwindow.h>

#include "generaldialogs.h"

#include "patheditor.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint patheditor_signals[LAST_SIGNAL] = { 0 };

static void patheditor_class_init (PathEditorClass *klass);
static void patheditor_init (PathEditor *sel);

static void patheditor_addpath(GtkWidget *button,gpointer user_data)
{
	PathEditor *pe=PATHEDITOR(user_data);

	char *dir=NULL;
	if((dir=Directory_Dialog(_("Select directory..."),NULL,NULL)))
	{
		GtkTreeIter iter1;
		gtk_list_store_append(pe->liststore,&iter1);
		gtk_list_store_set(pe->liststore,&iter1,0,dir,-1);
		g_signal_emit(G_OBJECT (pe),patheditor_signals[CHANGED_SIGNAL], 0);		
	}
}


static void patheditor_removepath(GtkWidget *button,gpointer user_data)
{
	PathEditor *pe=PATHEDITOR(user_data);

	GtkTreeIter iter;
	GtkTreeSelection *select;
	GtkTreeModel *model;

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (pe->treeview));
	if (gtk_tree_selection_get_selected (select, &model, &iter))
	{
		gtk_list_store_remove(pe->liststore,&iter);
		g_signal_emit(G_OBJECT (pe),patheditor_signals[CHANGED_SIGNAL], 0);
	}
}


GtkWidget*
patheditor_new (SearchPathHandler *sp)
{
	PathEditor *c=PATHEDITOR(g_object_new (patheditor_get_type (), NULL));

	c->liststore=gtk_list_store_new(1,G_TYPE_STRING);

	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (c), sw, TRUE, TRUE, 0);
	gtk_widget_show(sw);

	c->treeview=gtk_tree_view_new_with_model(GTK_TREE_MODEL(c->liststore));
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer=gtk_cell_renderer_text_new();
	column=gtk_tree_view_column_new_with_attributes(_("Path"),renderer,"text",0,NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(c->treeview),column);
	
	gtk_container_add(GTK_CONTAINER(sw),c->treeview);
	gtk_widget_show(c->treeview);

	patheditor_set_paths(c,sp);
	
	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(c),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);

	GtkWidget *addbutton,*removebutton;

	addbutton=gtk_button_new_with_label(_("Add..."));	
	gtk_box_pack_start(GTK_BOX(hbox),addbutton,TRUE,TRUE,0);
	g_signal_connect(addbutton,"clicked",G_CALLBACK(patheditor_addpath),c);
	gtk_widget_show(addbutton);
		
	removebutton=gtk_button_new_with_label(_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox),removebutton,TRUE,TRUE,0);
	g_signal_connect(removebutton,"clicked",G_CALLBACK(patheditor_removepath),c);
	gtk_widget_show(removebutton);

	return(GTK_WIDGET(c));
}


GType
patheditor_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo patheditor_info =
		{
			sizeof (PathEditorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) patheditor_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (PathEditor),
			0,
			(GInstanceInitFunc) patheditor_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "PathEditor", &patheditor_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void
patheditor_class_init (PathEditorClass *klass)
{
	patheditor_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (PathEditorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
patheditor_init (PathEditor *c)
{
	c->list=NULL;
	c->entry=NULL;
	c->pathlist=NULL;
	c->searchpathhandler=NULL;
}


gboolean patheditor_refresh(PathEditor *c)
{
	return(true);
}


void patheditor_get_paths(PathEditor *p,SearchPathHandler *sp)
{
	GtkTreeIter iter;
	gboolean valid;
	valid=gtk_tree_model_get_iter_first(GTK_TREE_MODEL(p->liststore),&iter);

	sp->ClearPaths();

	while(valid)
	{
		gchar *path;
		gtk_tree_model_get(GTK_TREE_MODEL(p->liststore),&iter,0,&path,-1);
		cerr << "Adding: " << path << endl;
		sp->AddPath(path);
		valid=gtk_tree_model_iter_next(GTK_TREE_MODEL(p->liststore),&iter);
	}
}


void patheditor_set_paths(PathEditor *p,SearchPathHandler *sp)
{
	p->searchpathhandler=sp;

	GtkTreeIter iter1;

	const char *path=sp->GetNextPath(NULL);
	while(path)
	{
		gtk_list_store_append(p->liststore,&iter1);
		gtk_list_store_set(p->liststore,&iter1,0,path,-1);
		path=sp->GetNextPath(path);
	}
}

