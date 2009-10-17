/*
 * effectselector.c - provides a list of available effects.
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
#include <gtk/gtkentry.h>
#include <gtk/gtklist.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtktreeselection.h>
#include <gtk/gtkscrolledwindow.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "../support/debug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../miscwidgets/generaldialogs.h"

#include "ppeffect.h"
#include "ppeffect_desaturate.h"
#include "ppeffect_temperature.h"

#include "effectlist.h"

#include "effectselector.h"

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

enum {
	CHANGED_SIGNAL,
	ADDEFFECT_SIGNAL,
	REMOVEEFFECT_SIGNAL,
	LAST_SIGNAL
};

static guint effectselector_signals[LAST_SIGNAL] = { 0 };

static void effectselector_class_init (EffectSelectorClass *klass);
static void effectselector_init (EffectSelector *sel);


static PPEffect *getcurrenteffect(EffectSelector *sel)
{
	PPEffect *result=NULL;
	if(sel->current)
	{
//		int item=effectselector_get_selected(sel);
		result=sel->current->Find(sel->available->GetID(sel->selected));
	}
	return(result);
}


static void effectwidget_changed(GtkWidget *wid,gpointer user_data)
{
	EffectSelector *pe=EFFECTSELECTOR(user_data);
	effectselector_apply(pe,pe->current);
	g_signal_emit(G_OBJECT (pe),effectselector_signals[CHANGED_SIGNAL], 0);
}


void effectselector_apply(EffectSelector *sel,PPEffectHeader *target)
{
	if(target)
	{
		target->ObtainMutex(); // Exclusive
		int item=effectselector_get_selected(sel);
		PPEffect *effect=target->Find(sel->available->GetID(item));
		if(effect)
		{
			sel->available->WidgetToEffect(sel->effectwidget,effect);
		}
		target->ReleaseMutex();
	}
}

static void selection_changed(GtkTreeSelection *select,gpointer user_data)
{
	EffectSelector *pe=EFFECTSELECTOR(user_data);
	int item=effectselector_get_selected(pe);
	pe->selected=item;

	if(pe->effectwidget)
		gtk_widget_destroy(pe->effectwidget);
	pe->effectwidget=NULL;

	PPEffect *effect=NULL;
	if(pe->current)
		effect=pe->current->Find(pe->available->GetID(item));
	pe->effectwidget=pe->available->CreateWidget(item,effect);
	if(pe->effectwidget)
	{
		gtk_box_pack_start(GTK_BOX(pe),pe->effectwidget,FALSE,FALSE,0);
		g_signal_connect (G_OBJECT (pe->effectwidget), "changed",
			G_CALLBACK (effectwidget_changed),pe);
		gtk_widget_show(pe->effectwidget);
	}
}



// Declare multiple columns for GTK's tree model.  The columnns can contain widgets,
// not just static display, and don't have to be visible, so we can include userdata.
enum
{
  ACTIVE_COLUMN = 0,	// Checkmark
  ICON_COLUMN,			// Pixbuf
  LABEL_COLUMN,			// Effect name (string)
						// Now non-displayed fields:
  EFFECTINDEX_COLUMN,		// Index of effect in EffectListSource.
  NUM_COLUMNS
};


static void populate_list(EffectSelector *es)
{
	GtkTreeIter iter;

	int c=es->available->EffectCount();
	for(int i=0;i<c;++i)
	{
		gtk_tree_store_append (GTK_TREE_STORE(es->treestore), &iter, NULL);
		gtk_tree_store_set (GTK_TREE_STORE(es->treestore), &iter,
			ACTIVE_COLUMN, FALSE,
			LABEL_COLUMN, es->available->GetName(i),
			ICON_COLUMN, es->available->GetIcon(i),
			EFFECTINDEX_COLUMN, i,
			-1);
    }
}


static void
item_toggled (GtkCellRendererToggle *cell,gchar *path_str,gpointer data)
{
	EffectSelector *sel=EFFECTSELECTOR(data);
	GtkTreeModel *model = GTK_TREE_MODEL(sel->treestore);
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeIter iter;
	gboolean toggle_item;

	// It seems the checkbox state has to be toggled manually by the callback...

	gint *column = (gint *)g_object_get_data (G_OBJECT (cell), "column");

	/* get toggled iter */
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, column, &toggle_item, -1);

	/* do something with the value */
	toggle_item ^= 1;

	/* set new value */
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,
		toggle_item, -1);

	// Housekeeping finished - can now process the result.

	// Set the current item
	int item;
	gtk_tree_model_get (model, &iter, EFFECTINDEX_COLUMN, &item, -1);
	sel->selected=item;

	// Here we either add the effect to the header, or remove it.

	if(toggle_item)
	{
		effectselector_add_selected_effect(sel,sel->current);
		if(sel->effectwidget)
			gtk_widget_set_sensitive(sel->effectwidget,TRUE);
		g_signal_emit(G_OBJECT (sel),effectselector_signals[ADDEFFECT_SIGNAL], 0);
	}
	else
	{
		effectselector_remove_selected_effect(sel,sel->current);
//		if(pe)
//		{
//			Debug[TRACE] << "About to delete item of type: " << pe->GetID() << endl;
//			delete pe;
//		}
		if(sel->effectwidget)
			gtk_widget_set_sensitive(sel->effectwidget,FALSE);
		g_signal_emit(G_OBJECT (sel),effectselector_signals[REMOVEEFFECT_SIGNAL], 0);
	}

	// Emit the Changed signal
	g_signal_emit(G_OBJECT (sel),effectselector_signals[CHANGED_SIGNAL], 0);

	// Clean up
	gtk_tree_path_free (path);
}


static void setup_renderers(EffectSelector *sel)
{
	gint col_offset;
	GtkCellRenderer *renderer;

	// Active column
	renderer = gtk_cell_renderer_toggle_new ();

	g_signal_connect (renderer, "toggled", G_CALLBACK (item_toggled), sel);

	col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (sel->treeview),
		-1, _("Active"),
		renderer,
		"active",
		ACTIVE_COLUMN,
		NULL);

	// Pixbuf column
	renderer=gtk_cell_renderer_pixbuf_new();
	col_offset=gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW (sel->treeview),
		-1,_("Icon"),
		renderer,
		"pixbuf",
		ICON_COLUMN,
		NULL);

	// column for names
	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.0, NULL);
	g_object_set (renderer, "xpad", 10, NULL);
	
	col_offset = gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (sel->treeview),
	    -1, _("Name"),
	    renderer, "text",
	    LABEL_COLUMN,
	    NULL);
}


GtkWidget*
effectselector_new ()
{
	EffectSelector *c=EFFECTSELECTOR(g_object_new (effectselector_get_type (), NULL));
	c->available=new EffectListSource;

	c->treestore = gtk_tree_store_new (NUM_COLUMNS,
		G_TYPE_BOOLEAN,
		GDK_TYPE_PIXBUF,
		G_TYPE_STRING,
		G_TYPE_INT,
		G_TYPE_POINTER
		);

	GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (c), sw, TRUE, TRUE, 0);
	gtk_widget_show(sw);

	c->treeview=gtk_tree_view_new_with_model(GTK_TREE_MODEL(c->treestore));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(c->treeview),false);
	setup_renderers(c);

	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (c->treeview));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed",
		G_CALLBACK (selection_changed),c);

	gtk_container_add(GTK_CONTAINER(sw),c->treeview);
	gtk_widget_show(c->treeview);

	GtkWidget *hbox=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(c),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);

	gtk_widget_set_sensitive(GTK_WIDGET(c),false);

	populate_list(c);
	
	return(GTK_WIDGET(c));
}


GType
effectselector_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo effectselector_info =
		{
			sizeof (EffectSelectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) effectselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (EffectSelector),
			0,
			(GInstanceInitFunc) effectselector_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "EffectSelector", &effectselector_info, GTypeFlags(0));
	}
	return stpuic_type;
}


static void *parent_class=NULL;

static void effectselector_destroy(GtkObject *object)
{
	if(object && IS_EFFECTSELECTOR(object))
	{
		EffectSelector *es=EFFECTSELECTOR(object);
		if(es->available)
			delete es->available;
		es->available=NULL;
		if (GTK_OBJECT_CLASS (parent_class)->destroy)
			(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
	}
}


static void
effectselector_class_init (EffectSelectorClass *cls)
{
	GtkObjectClass *object_class=(GtkObjectClass *)cls;

	parent_class = gtk_type_class (gtk_widget_get_type ());

	object_class->destroy = effectselector_destroy;

	effectselector_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectSelectorClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	effectselector_signals[ADDEFFECT_SIGNAL] =
	g_signal_new ("addeffect",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectSelectorClass, addeffect),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	effectselector_signals[REMOVEEFFECT_SIGNAL] =
	g_signal_new ("removeeffect",
		G_TYPE_FROM_CLASS (cls),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (EffectSelectorClass, removeeffect),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
effectselector_init (EffectSelector *c)
{
	c->available=NULL;
	c->current=NULL;
	c->effectwidget=NULL;
}


gboolean effectselector_refresh(EffectSelector *c)
{
	return(true);
}


int effectselector_get_selected(EffectSelector *pe)
{
	GtkTreeSelection *select;
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (pe->treeview));

	int i=0;
	GtkTreeIter iter;
	GtkTreeModel *model;
	if (gtk_tree_selection_get_selected (select,&model, &iter))
		gtk_tree_model_get (model, &iter, EFFECTINDEX_COLUMN, &i, -1);

	return(i);
}

// This will be called for each entry in the "available" list.
// We need to check against the current list, and mark the
// checkboxes accordingly.

static gboolean set_current_list_foreach_func(GtkTreeModel *model,GtkTreePath *path,GtkTreeIter *iter,gpointer data)
{
	EffectSelector *es=EFFECTSELECTOR(data);
	int i;
	gtk_tree_model_get (model, iter, EFFECTINDEX_COLUMN, &i, -1);

	PPEffect *pe=es->current->Find(es->available->GetID(i));
	if(pe)
	{
		gtk_tree_store_set (GTK_TREE_STORE (model), iter, ACTIVE_COLUMN, TRUE, -1);
	}
	else
	{
		gtk_tree_store_set (GTK_TREE_STORE (model), iter, ACTIVE_COLUMN, FALSE, -1);
	}

	return(false);
}

void effectselector_set_current_list(EffectSelector *es,PPEffectHeader *current)
{
	es->current=current;
	if(current)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(es),TRUE);
		gtk_tree_model_foreach(GTK_TREE_MODEL(es->treestore),set_current_list_foreach_func,es);
	}
	else
	{
		// Loop through and set all checkboxes to FALSE?
		gtk_widget_set_sensitive(GTK_WIDGET(es),FALSE);
	}
	if(es->effectwidget)
	{
		PPEffect *effect=getcurrenteffect(es);
		if(effect)
		{
			gtk_widget_set_sensitive(GTK_WIDGET(es->effectwidget),(current!=NULL));
			es->available->EffectToWidget(es->effectwidget,effect);
		}
		else
			gtk_widget_set_sensitive(GTK_WIDGET(es->effectwidget),FALSE);
	}
}


PPEffect *effectselector_add_selected_effect(EffectSelector *es,PPEffectHeader *chain)
{
	PPEffect *result=NULL;
	chain->ObtainMutex(); // Exclusive
	if(chain)
	{
		if(chain->Find(es->available->GetID(es->selected)))
			Debug[TRACE] << "Effect already present - skipping..." << endl;
		else
		{
			Debug[TRACE] << "About to create a new item of type: " << es->available->GetID(es->selected) << endl;
			result=es->available->CreateEffect(es->selected,*chain);
			Debug[TRACE] << "Done" << endl;
		}
	}
	chain->ReleaseMutex();
	return(result);
}


void effectselector_remove_selected_effect(EffectSelector *es,PPEffectHeader *chain)
{
	if(chain)
	{
		Debug[TRACE] << "RemoveEffect: Obtain" << endl;
		chain->ObtainMutex(); // Exclusive
		PPEffect *pe=chain->Find(es->available->GetID(es->selected));
		if(pe)
			delete pe;
		else
			Debug[WARN] << "Effect not found - not deleting!" << endl;
		Debug[TRACE] << "RemoveEffect: Release" << endl;
		chain->ReleaseMutex();
	}
}
