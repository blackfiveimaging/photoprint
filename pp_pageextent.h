#ifndef __PP_PAGEEXTENT_H__
#define __PP_PAGEEXTENT_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtksizegroup.h>

#include <gutenprint/gutenprint.h>

#include "pageextent.h"

G_BEGIN_DECLS

#define PP_PAGEEXTENT_TYPE			(pp_pageextent_get_type())
#define PP_PAGEEXTENT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_PAGEEXTENT_TYPE, pp_PageExtent))
#define PP_PAGEEXTENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_PAGEEXTENT_TYPE, pp_PageExtentClass))
#define IS_PP_PAGEEXTENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_PAGEEXTENT_TYPE))
#define IS_PP_PAGEEXTENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_PAGEEXTENT_TYPE))

typedef struct _pp_PageExtent pp_PageExtent;
typedef struct _pp_PageExtentClass pp_PageExtentClass;

class PhotoPrint_State;

struct _pp_PageExtent
{
	GtkExpander	box;
	GtkWidget *lmargin;
	GtkWidget *rmargin;
	GtkWidget *tmargin;
	GtkWidget *bmargin;
	GtkWidget *pagesize;
	GtkWidget *customwidth;
	GtkWidget *customwidthlabel;
	GtkWidget *customheight;
	GtkWidget *customheightlabel;
	PageExtent *pe;
	PhotoPrint_State *state;
	bool blocksignals;
};


struct _pp_PageExtentClass
{
	GtkExpanderClass parent_class;

	void (*changed)(pp_PageExtent *sig);
};

GType pp_pageextent_get_type (void);
GtkWidget* pp_pageextent_new (PageExtent *pe,PhotoPrint_State *state);
void pp_pageextent_refresh(pp_PageExtent *ob);
void pp_pageextent_set_unit(pp_PageExtent *ob,enum Units unit);

G_END_DECLS

#endif /* __PP_PAGEEXTENT_H__ */
