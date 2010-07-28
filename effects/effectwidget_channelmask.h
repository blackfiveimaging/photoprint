#ifndef __EFFECTWIDGET_CHANNELMASK_H__
#define __EFFECTWIDGET_CHANNELMASK_H__


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

#include "devicencolorant.h"

G_BEGIN_DECLS

#define EFFECTWIDGET_CHANNELMASK_TYPE			(effectwidget_channelmask_get_type())
#define EFFECTWIDGET_CHANNELMASK(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EFFECTWIDGET_CHANNELMASK_TYPE, EffectWidget_ChannelMask))
#define EFFECTWIDGET_CHANNELMASK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EFFECTWIDGET_CHANNELMASK_TYPE, EffectWidget_ChannelMaskClass))
#define IS_EFFECTWIDGET_CHANNELMASK(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EFFECTWIDGET_CHANNELMASK_TYPE))
#define IS_EFFECTWIDGET_CHANNELMASK_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EFFECTWIDGET_CHANNELMASK_TYPE))

typedef struct _EffectWidget_ChannelMask EffectWidget_ChannelMask;
typedef struct _EffectWidget_ChannelMaskClass EffectWidget_ChannelMaskClass;

class EffectListSource;

struct _EffectWidget_ChannelMask
{
	GtkVBox box;
	GtkWidget *channels;
	GtkWidgetClass *parent_class;
};


struct _EffectWidget_ChannelMaskClass
{
	GtkVBoxClass parent_class;

	void (*changed)(EffectWidget_ChannelMask *combo);
};

GType effectwidget_channelmask_get_type (void);

GtkWidget* effectwidget_channelmask_new(DeviceNColorantList *colorants);
//double effectwidget_channelmask_get(EffectWidget_ChannelMask *pe);
void effectwidget_channelmask_set(EffectWidget_ChannelMask *es,DeviceNColorantList *colorants);

G_END_DECLS

#endif /* __EFFECTWIDGET_CHANNELMASK_H__ */
