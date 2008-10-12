/*
 * stpui_queue.c - provides a custom widget for selecting a
 * printer queue.
 *
 * Copyright (c) 2006 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */


#include <string.h>
#include <gtk/gtk.h>
#include "../stp_support/printerqueues.h"

#include "stpui_queue.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint stpui_queue_signals[LAST_SIGNAL] = { 0 };

static void stpui_queue_class_init (stpui_QueueClass *klass);
static void stpui_queue_init (stpui_Queue *t);


static void stpui_queue_changed(GtkWidget *wid,gpointer *ud)
{
	stpui_Queue *t=STPUI_QUEUE(ud);

	int entry;
	int customcommandenabled=FALSE;

	entry=gtk_combo_box_get_active(GTK_COMBO_BOX(t->queue));

	if(entry>-1)
	{
		char *queue;
		queue=t->pq->GetPrinterName(t->pq,entry);
		fprintf(stderr,"Currently selected: %s\n",queue);
		t->pq->SetPrinterQueue(t->pq,queue);
		
		char *cmd=t->pq->GetDriver(t->pq);
		if(cmd)
			fprintf(stderr,"Got driver: %s\n",cmd);
		
		if(strcmp(queue,PRINTERQUEUE_CUSTOMCOMMAND)==0)
			customcommandenabled=TRUE;
	}

#ifndef WIN32
	if(customcommandenabled)
		gtk_widget_show(t->customcommand);
	else
		gtk_widget_hide(t->customcommand);
#endif

	g_signal_emit(G_OBJECT (t),
		stpui_queue_signals[CHANGED_SIGNAL], 0);
}


#ifndef WIN32
static void stpui_customcommand_changed(GtkWidget *wid,gpointer *ud)
{
	stpui_Queue *t=STPUI_QUEUE(ud);
	const char *cc=gtk_entry_get_text(GTK_ENTRY(t->customcommand));
	if(cc && strlen(cc)==0)
		cc=NULL;
	t->pq->SetCustomCommand(t->pq,cc);
	
	g_signal_emit(G_OBJECT (t),
		stpui_queue_signals[CHANGED_SIGNAL], 0);
}
#endif

GtkWidget*
stpui_queue_new (struct pqinfo *pq)
{
	stpui_Queue *c=STPUI_QUEUE(g_object_new (stpui_queue_get_type (), NULL));
	int count,j;

	c->pq=pq;
	
	c->queue=gtk_combo_box_new_text();
	gtk_box_pack_start(GTK_BOX(&c->box),GTK_WIDGET(c->queue),TRUE,TRUE,0);
	gtk_widget_show(c->queue);

	count=pq->GetPrinterCount(pq);
	
	const char *current=pq->GetPrinterQueue(pq);
	int active=-1;
	for(j=0;j<count;++j)
	{
		char *queuename=pq->GetPrinterName(pq,j);
		if(current && strcmp(current,queuename)==0)
			active=j;
		gtk_combo_box_append_text(GTK_COMBO_BOX(c->queue),queuename);
		free(queuename);
	}
	if(j>-1)
		gtk_combo_box_set_active(GTK_COMBO_BOX(c->queue),active);

	g_signal_connect(GTK_WIDGET(c->queue),"changed",G_CALLBACK(stpui_queue_changed),c);

#ifndef WIN32
	const char *cc=pq->GetCustomCommand(pq);
	c->customcommand=gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(&c->box),GTK_WIDGET(c->customcommand),TRUE,TRUE,0);
	if(cc)
		gtk_entry_set_text(GTK_ENTRY(c->customcommand),cc);
	g_signal_connect(GTK_WIDGET(c->customcommand),"changed",G_CALLBACK(stpui_customcommand_changed),c);

	int entry=gtk_combo_box_get_active(GTK_COMBO_BOX(c->queue));
	if(entry>-1)
	{
		char *queue=pq->GetPrinterName(pq,entry);
		if(strcmp(queue,PRINTERQUEUE_CUSTOMCOMMAND)==0)
			gtk_widget_show(c->customcommand);
	}
#endif

	
	return(GTK_WIDGET(c));
}


GType
stpui_queue_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo stpui_queue_info =
		{
			sizeof (stpui_QueueClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) stpui_queue_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (stpui_Queue),
			0,
			(GInstanceInitFunc) stpui_queue_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "stpui_Queue", &stpui_queue_info, 0);
	}
	return stpuic_type;
}


static void
stpui_queue_class_init (stpui_QueueClass *klass)
{
	stpui_queue_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (stpui_QueueClass, queued),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
stpui_queue_init (stpui_Queue *c)
{
}


gboolean stpui_queue_refresh(stpui_Queue *t)
{
	gboolean result=TRUE;

//	g_signal_handlers_block_matched (G_OBJECT (t->queue), 
//                                        G_SIGNAL_MATCH_DATA,
//                                         0, 0, NULL, NULL, t);

//	result=stpui_queue_set_state(t);

//	g_signal_handlers_unblock_matched (G_OBJECT (t->queue), 
//                                         G_SIGNAL_MATCH_DATA,
//                                         0, 0, NULL, NULL, t);
	return(result);
}
