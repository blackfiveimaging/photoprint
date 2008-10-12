#ifndef __EFFECTWIDGET_UNSHARPMASK_H__
#define __EFFECTWIDGET_UNSHARPMASK_H__


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

#define EFFECTWIDGET_UNSHARPMASK_TYPE			(effectwidget_unsharpmask_get_type())
#define EFFECTWIDGET_UNSHARPMASK(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EFFECTWIDGET_UNSHARPMASK_TYPE, EffectWidget_UnsharpMask))
#define EFFECTWIDGET_UNSHARPMASK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EFFECTWIDGET_UNSHARPMASK_TYPE, EffectWidget_UnsharpMaskClass))
#define IS_EFFECTWIDGET_UNSHARPMASK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EFFECTWIDGET_UNSHARPMASK_TYPE))
#define IS_EFFECTWIDGET_UNSHARPMASK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EFFECTWIDGET_UNSHARPMASK_TYPE))

typedef struct _EffectWidget_UnsharpMask EffectWidget_UnsharpMask;
typedef struct _EffectWidget_UnsharpMaskClass EffectWidget_UnsharpMaskClass;

class EffectListSource;

struct _EffectWidget_UnsharpMask
{
	GtkVBox box;
	GtkWidget *radiusslider;
	GtkWidget *amountslider;
	GtkWidgetClass *parent_class;
};


struct _EffectWidget_UnsharpMaskClass
{
	GtkVBoxClass parent_class;

	void (*changed)(EffectWidget_UnsharpMask *combo);
};

GType effectwidget_unsharpmask_get_type (void);

GtkWidget* effectwidget_unsharpmask_new();
float effectwidget_unsharpmask_get_radius(EffectWidget_UnsharpMask *pe);
void effectwidget_unsharpmask_set_radius(EffectWidget_UnsharpMask *es,float radius);
float effectwidget_unsharpmask_get_amount(EffectWidget_UnsharpMask *pe);
void effectwidget_unsharpmask_set_amount(EffectWidget_UnsharpMask *es,float amount);

G_END_DECLS

#endif /* __EFFECTWIDGET_UNSHARPMASK_H__ */
