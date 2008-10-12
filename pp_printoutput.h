#ifndef __PP_PRINTOUTPUT_H__
#define __PP_PRINTOUTPUT_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkspinbutton.h>

#include <gutenprint/gutenprint.h>

#include "printoutput.h"

G_BEGIN_DECLS

#define PP_PRINTOUTPUT_TYPE			(pp_printoutput_get_type())
#define PP_PRINTOUTPUT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_PRINTOUTPUT_TYPE, pp_PrintOutput))
#define PP_PRINTOUTPUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_PRINTOUTPUT_TYPE, pp_PrintOutputClass))
#define IS_PP_PRINTOUTPUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_PRINTOUTPUT_TYPE))
#define IS_PP_PRINTOUTPUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_PRINTOUTPUT_TYPE))

typedef struct _pp_PrintOutput pp_PrintOutput;
typedef struct _pp_PrintOutputClass pp_PrintOutputClass;

struct _pp_PrintOutput
{
	GtkVBox	box;
	GtkWidget *string;
	GtkWidget *combo;
	GtkWidget *printersel;
	GList *queues;
	PrintOutput *po;
};


struct _pp_PrintOutputClass
{
	GtkVBoxClass parent_class;

	void (*changed)(pp_PrintOutput *book);
};

GType pp_printoutput_get_type (void);
GtkWidget* pp_printoutput_new (PrintOutput *po);
void pp_printoutput_refresh(pp_PrintOutput *ob);

void pp_printoutput_queue_dialog(PrintOutput *po);

G_END_DECLS

#endif /* __PP_PRINTOUTPUT_H__ */
