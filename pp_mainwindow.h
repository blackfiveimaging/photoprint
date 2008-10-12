#ifndef __PP_MAINWINDOW_H__
#define __PP_MAINWINDOW_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtkuimanager.h>

#include <gutenprint/gutenprint.h>

#include "photoprint_state.h"

G_BEGIN_DECLS

#define PP_MAINWINDOW_TYPE			(pp_mainwindow_get_type())
#define PP_MAINWINDOW(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_MAINWINDOW_TYPE, pp_MainWindow))
#define PP_MAINWINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_MAINWINDOW_TYPE, pp_MainWindowClass))
#define IS_PP_MAINWINDOW(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_MAINWINDOW_TYPE))
#define IS_PP_MAINWINDOW_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_MAINWINDOW_TYPE))

typedef struct _pp_MainWindow pp_MainWindow;
typedef struct _pp_MainWindowClass pp_MainWindowClass;

struct _pp_MainWindow
{
	GtkWindow window;
//	GtkWidget *menu;
	GtkWidget *layout;
	GtkWidget *vbox;
	GtkUIManager *uim;
	PhotoPrint_State *state;
	char *prevfile;
};


struct _pp_MainWindowClass
{
	GtkWindowClass parent_class;

	void (*changed)(pp_MainWindow *book);
	void (*startup)(pp_MainWindow *book);
	void (*initialised)(pp_MainWindow *book);
};

GType pp_mainwindow_get_type (void);
GtkWidget* pp_mainwindow_new (PhotoPrint_State *state);
void pp_mainwindow_refresh(pp_MainWindow *ob);
void pp_mainwindow_rebuild(pp_MainWindow *ob);

G_END_DECLS

#endif /* __PP_MAINWINDOW_H__ */
