#ifndef __EFFECTWIDGET_GAMMA_H__
#define __EFFECTWIDGET_GAMMA_H__


#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreestore.h>
#include <gtk/gtkcellrendererpixbuf.h>


G_BEGIN_DECLS

#define EFFECTWIDGET_GAMMA_TYPE			(effectwidget_gamma_get_type())
#define EFFECTWIDGET_GAMMA(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EFFECTWIDGET_GAMMA_TYPE, EffectWidget_Gamma))
#define EFFECTWIDGET_GAMMA_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EFFECTWIDGET_GAMMA_TYPE, EffectWidget_GammaClass))
#define IS_EFFECTWIDGET_GAMMA(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EFFECTWIDGET_GAMMA_TYPE))
#define IS_EFFECTWIDGET_GAMMA_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EFFECTWIDGET_GAMMA_TYPE))

typedef struct _EffectWidget_Gamma EffectWidget_Gamma;
typedef struct _EffectWidget_GammaClass EffectWidget_GammaClass;

class EffectListSource;

struct _EffectWidget_Gamma
{
	GtkVBox box;
	GtkWidget *slider;
	GtkWidgetClass *parent_class;
};


struct _EffectWidget_GammaClass
{
	GtkVBoxClass parent_class;

	void (*changed)(EffectWidget_Gamma *combo);
};

GType effectwidget_gamma_get_type (void);

GtkWidget* effectwidget_gamma_new();
double effectwidget_gamma_get(EffectWidget_Gamma *pe);
void effectwidget_gamma_set(EffectWidget_Gamma *es,double gamma);

G_END_DECLS

#endif /* __EFFECTWIDGET_GAMMA_H__ */
