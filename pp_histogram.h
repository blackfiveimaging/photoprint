#ifndef __PP_HISTOGRAM_H__
#define __PP_HISTOGRAM_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>

#include "layout.h"

G_BEGIN_DECLS

#define PP_HISTOGRAM_TYPE			(pp_histogram_get_type())
#define PP_HISTOGRAM(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_HISTOGRAM_TYPE, pp_Histogram))
#define PP_HISTOGRAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_HISTOGRAM_TYPE, pp_HistogramClass))
#define IS_PP_HISTOGRAM(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_HISTOGRAM_TYPE))
#define IS_PP_HISTOGRAM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_HISTOGRAM_TYPE))

typedef struct _pp_Histogram pp_Histogram;
typedef struct _pp_HistogramClass pp_HistogramClass;

struct Signature;
class Thread;
struct _pp_Histogram
{
	GtkExpander	expander;
	GtkWidget *hist;
	GtkWidget *scrollwin;
//	ThreadFunction *thread;
	Layout *layout;
};


struct _pp_HistogramClass
{
	GtkExpanderClass parent_class;

	void (*changed)(pp_Histogram *ii);
};

GType pp_histogram_get_type (void);
GtkWidget* pp_histogram_new (Layout *layout);
void pp_histogram_refresh(pp_Histogram *ob);
void pp_histogram_change_image(pp_Histogram *ob);
G_END_DECLS

#endif /* __PP_HISTOGRAM_H__ */
