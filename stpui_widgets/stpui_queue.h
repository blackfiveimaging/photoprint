#ifndef __STPUI_QUEUE_H__
#define __STPUI_QUEUE_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkvbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_QUEUE_TYPE			(stpui_queue_get_type())
#define STPUI_QUEUE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_QUEUE_TYPE, stpui_Queue))
#define STPUI_QUEUE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_QUEUE_TYPE, stpui_QueueClass))
#define IS_STPUI_QUEUE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_QUEUE_TYPE))
#define IS_STPUI_QUEUE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_QUEUE_TYPE))

typedef struct _stpui_Queue stpui_Queue;
typedef struct _stpui_QueueClass stpui_QueueClass;

struct _stpui_Queue
{
	GtkVBox box;
	GtkWidget *queue;
	GtkWidget *customcommand;
	struct pqinfo *pq;
};


struct _stpui_QueueClass
{
	GtkVBoxClass parent_class;

	void (*queued)(stpui_Queue *t);
};

GType stpui_queue_get_type (void);
GtkWidget* stpui_queue_new (struct pqinfo *pq);
gboolean stpui_queue_refresh(stpui_Queue *t);

G_END_DECLS

#endif /* __STPUI_QUEUE_H__ */
