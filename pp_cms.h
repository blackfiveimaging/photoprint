#ifndef __PP_CMS_H__
#define __PP_CMS_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkspinbutton.h>

#include "profilemanager/lcmswrapper.h"
#include "support/configdb.h"
#include "photoprint_state.h"

G_BEGIN_DECLS

#define PP_CMS_TYPE			(pp_cms_get_type())
#define PP_CMS(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), PP_CMS_TYPE, pp_CMS))
#define PP_CMS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PP_CMS_TYPE, pp_CMSClass))
#define IS_PP_CMS(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PP_CMS_TYPE))
#define IS_PP_CMS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PP_CMS_TYPE))

typedef struct _pp_CMS pp_CMS;
typedef struct _pp_CMSClass pp_CMSClass;

struct _pp_CMS
{
	GtkVBox	box;
	ProfileManager *pm;
	GtkWidget *intent;
	GtkWidget *colourspace;
	GtkWidget *printeractive;
	GtkWidget *printerprof;
	GtkWidget *rgbactive;
	GtkWidget *rgbprof;
	GtkWidget *cmykactive;
	GtkWidget *cmykprof;
	GtkWidget *monitoractive;
	GtkWidget *monitorprof;
	GtkWidget *displaymode;
	GtkWidget *indicator[3];
	GtkWidget *statusline[3];
};


struct _pp_CMSClass
{
	GtkVBoxClass parent_class;

	void (*changed)(pp_CMS *book);
};

GType pp_cms_get_type (void);
GtkWidget* pp_cms_new (ProfileManager *pm);
void pp_cms_refresh(pp_CMS *ob);
void pp_cms_populate(pp_CMS *ob,PhotoPrint_State *db);
void pp_cms_depopulate(pp_CMS *ob,PhotoPrint_State *db);
void pp_cms_run_dialog(PhotoPrint_State *db,GtkWindow *parent);

G_END_DECLS

#endif /* __PP_CMS_H__ */
