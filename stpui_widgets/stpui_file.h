#ifndef __STPUI_FILE_H__
#define __STPUI_FILE_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>

#include <gutenprint/gutenprint.h>

G_BEGIN_DECLS

#define STPUI_FILE_TYPE			(stpui_file_get_type())
#define STPUI_FILE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), STPUI_FILE_TYPE, stpui_File))
#define STPUI_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), STPUI_FILE_TYPE, stpui_FileClass))
#define IS_STPUI_FILE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), STPUI_FILE_TYPE))
#define IS_STPUI_FILE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), STPUI_FILE_TYPE))

typedef struct _stpui_File stpui_File;
typedef struct _stpui_FileClass stpui_FileClass;

struct _stpui_File
{
	GtkHBox box;
	GtkWidget *entry;
	GtkWidget *button;
	stp_vars_t *vars;
	const char *optionname;
};


struct _stpui_FileClass
{
	GtkHBoxClass parent_class;

	void (*changed)(stpui_File *t);
};

GType stpui_file_get_type (void);
GtkWidget* stpui_file_new (stp_vars_t *vars,const char *optname,const char *displayname);
gboolean stpui_file_refresh(stpui_File *t);

#define STPUI_FILE_DEFAULT_TOKEN "<default>"

G_END_DECLS

#endif /* __STPUI_FILE_H__ */
