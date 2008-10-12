#ifndef __IMAGESELECTOR_H__
#define __IMAGESELECTOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrendererpixbuf.h>

#include "searchpath.h"

G_BEGIN_DECLS

#define IMAGESELECTOR_TYPE			(imageselector_get_type())
#define IMAGESELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), IMAGESELECTOR_TYPE, ImageSelector))
#define IMAGESELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), IMAGESELECTOR_TYPE, ImageSelectorClass))
#define IS_IMAGESELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), IMAGESELECTOR_TYPE))
#define IS_IMAGESELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), IMAGESELECTOR_TYPE))

typedef struct _ImageSelector ImageSelector;
typedef struct _ImageSelectorClass ImageSelectorClass;

struct _ImageSelector
{
	GtkVBox box;
	GtkListStore *liststore;
	GtkWidget *treeview;
	GList *imagelist;
	SearchPathHandler *searchpath;
	GtkWidgetClass *parent_class;
	char *filename;
};


struct _ImageSelectorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(ImageSelector *combo);
};

GType imageselector_get_type (void);

GtkWidget* imageselector_new (SearchPathHandler *pm,bool allowselection=true);

gboolean imageselector_refresh(ImageSelector *c);
const char *imageselector_get_filename(ImageSelector *c);
void imageselector_set_filename(ImageSelector *c,const char *filename);

G_END_DECLS

#endif /* __IMAGESELECTOR_H__ */
