#ifndef __PATHEDITOR_H__
#define __PATHEDITOR_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>

#include "searchpath.h"

G_BEGIN_DECLS

#define PATHEDITOR_TYPE			(patheditor_get_type())
#define PATHEDITOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PATHEDITOR_TYPE, PathEditor))
#define PATHEDITOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PATHEDITOR_TYPE, PathEditorClass))
#define IS_PATHEDITOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PATHEDITOR_TYPE))
#define IS_PATHEDITOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PATHEDITOR_TYPE))

typedef struct _PathEditor PathEditor;
typedef struct _PathEditorClass PathEditorClass;

struct _PathEditor
{
	GtkVBox box;
	GtkWidget *list;
	GtkWidget *entry;
	GtkListStore *liststore;
	GtkWidget *treeview;
	GList *pathlist;
	SearchPathHandler *searchpathhandler;
};


struct _PathEditorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(PathEditor *combo);
};

GType patheditor_get_type (void);

GtkWidget* patheditor_new (SearchPathHandler *pm);

void patheditor_get_paths(PathEditor *p,SearchPathHandler *sp);
void patheditor_set_paths(PathEditor *p,SearchPathHandler *sp);

gboolean patheditor_refresh(PathEditor *c);

G_END_DECLS

#endif /* __PATHEDITOR_H__ */
