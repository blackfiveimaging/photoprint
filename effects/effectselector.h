#ifndef __EFFECTSELECTOR_H__
#define __EFFECTSELECTOR_H__


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

#define EFFECTSELECTOR_TYPE			(effectselector_get_type())
#define EFFECTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj), EFFECTSELECTOR_TYPE, EffectSelector))
#define EFFECTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), EFFECTSELECTOR_TYPE, EffectSelectorClass))
#define IS_EFFECTSELECTOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), EFFECTSELECTOR_TYPE))
#define IS_EFFECTSELECTOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), EFFECTSELECTOR_TYPE))

typedef struct _EffectSelector EffectSelector;
typedef struct _EffectSelectorClass EffectSelectorClass;

class EffectListSource;

struct _EffectSelector
{
	GtkVBox box;
	GtkTreeStore *treestore;
	GtkWidget *treeview;
	EffectListSource *available;
	PPEffectHeader *current;
	int selected;
	GtkWidget *effectwidget;
	GtkWidgetClass *parent_class;
};


struct _EffectSelectorClass
{
	GtkVBoxClass parent_class;

	void (*changed)(EffectSelector *es);
	void (*addeffect)(EffectSelector *es);
	void (*removeeffect)(EffectSelector *es);
};

GType effectselector_get_type (void);

GtkWidget* effectselector_new();
int effectselector_get_selected(EffectSelector *pe);
void effectselector_set_current_list(EffectSelector *es,PPEffectHeader *current);
gboolean effectselector_refresh(EffectSelector *c);

// The effectselector's operation is autonomous provided you only need to
// manipulate a single effect chain.  If you need to manipulate multiple effect
// chains - say, you have multiple images selected - then these functions
// will help.

// After receiving the "changed" signal, call this function on each effect chain
// in turn...

void effectselector_apply(EffectSelector *es,PPEffectHeader *target);

// and this after receiving the "addeffect" signal...

PPEffect *effectselector_add_selected_effect(EffectSelector *es,PPEffectHeader *target);

// and this after receiving the "removeeffect" signal...

void effectselector_remove_selected_effect(EffectSelector *es,PPEffectHeader *target);


G_END_DECLS

#endif /* __EFFECTSELECTOR_H__ */
