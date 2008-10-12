#ifndef __EFFECTWIDGET_TEMPCHANGE_H__
#define __EFFECTWIDGET_TEMPCHANGE_H__


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

#define EFFECTWIDGET_TEMPCHANGE_TYPE			(effectwidget_tempchange_get_type())
#define EFFECTWIDGET_TEMPCHANGE(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EFFECTWIDGET_TEMPCHANGE_TYPE, EffectWidget_TempChange))
#define EFFECTWIDGET_TEMPCHANGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EFFECTWIDGET_TEMPCHANGE_TYPE, EffectWidget_TempChangeClass))
#define IS_EFFECTWIDGET_TEMPCHANGE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EFFECTWIDGET_TEMPCHANGE_TYPE))
#define IS_EFFECTWIDGET_TEMPCHANGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EFFECTWIDGET_TEMPCHANGE_TYPE))

typedef struct _EffectWidget_TempChange EffectWidget_TempChange;
typedef struct _EffectWidget_TempChangeClass EffectWidget_TempChangeClass;

class EffectListSource;

struct _EffectWidget_TempChange
{
	GtkVBox box;
	GtkWidget *slider;
	GtkWidgetClass *parent_class;
};


struct _EffectWidget_TempChangeClass
{
	GtkVBoxClass parent_class;

	void (*changed)(EffectWidget_TempChange *combo);
};

GType effectwidget_tempchange_get_type (void);

GtkWidget* effectwidget_tempchange_new();
int effectwidget_tempchange_get(EffectWidget_TempChange *pe);
void effectwidget_tempchange_set(EffectWidget_TempChange *es,int temp);

G_END_DECLS

#endif /* __EFFECTWIDGET_TEMPCHANGE_H__ */
