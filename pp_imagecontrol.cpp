#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>

#include "layout.h"
#include "photoprint_state.h"

#include "debug.h"

#include "effects/ppeffect.h"
#include "effects/effectselector.h"
#include "pp_imageinfo.h"
#include "pp_histogram.h"

#include "pp_imagecontrol.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_imagecontrol_signals[LAST_SIGNAL] = { 0 };

static void pp_imagecontrol_class_init (pp_ImageControlClass *klass);
static void pp_imagecontrol_init (pp_ImageControl *stpuicombo);


static void effectselector_changed(GtkWidget *wid,gpointer *ob)
{
	pp_ImageControl *ic=(pp_ImageControl *)ob;
	LayoutIterator it(*ic->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		effectselector_apply(EFFECTSELECTOR(ic->effectselector),ii);
		ii->FlushThumbnail();
		ii=it.NextSelected();
	}
	g_signal_emit(G_OBJECT (ic),pp_imagecontrol_signals[CHANGED_SIGNAL], 0);
}


static void effectselector_addeffect(GtkWidget *wid,gpointer *ob)
{
	Debug[TRACE] << "Acting on addeffect signal" << endl;
	pp_ImageControl *ic=(pp_ImageControl *)ob;
	LayoutIterator it(*ic->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		effectselector_add_selected_effect(EFFECTSELECTOR(ic->effectselector),ii);
		ii=it.NextSelected();
	}
	g_signal_emit(G_OBJECT (ic),pp_imagecontrol_signals[CHANGED_SIGNAL], 0);
}


static void effectselector_removeeffect(GtkWidget *wid,gpointer *ob)
{
	Debug[TRACE] << "Acting on removeeffect signal" << endl;
	pp_ImageControl *ic=(pp_ImageControl *)ob;
	LayoutIterator it(*ic->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	while(ii)
	{
		effectselector_remove_selected_effect(EFFECTSELECTOR(ic->effectselector),ii);
		ii=it.NextSelected();
	}
	g_signal_emit(G_OBJECT (ic),pp_imagecontrol_signals[CHANGED_SIGNAL], 0);
}


void pp_imagecontrol_refresh(pp_ImageControl *ob)
{
	Debug[TRACE] << "pp_imagecontrol_refresh: refreshing imageinfo" << endl;
	pp_imageinfo_refresh(PP_IMAGEINFO(ob->imageinfo));
	Debug[TRACE] << "pp_imagecontrol_refresh: refreshing histogram" << endl;
	pp_histogram_refresh(PP_HISTOGRAM(ob->histogram));
	Debug[TRACE] << "pp_imagecontrol_refresh: done" << endl;
}


void pp_imagecontrol_set_image(pp_ImageControl *ob)
{
	LayoutIterator it(*ob->layout);
	Layout_ImageInfo *ii=it.FirstSelected();
	pp_imageinfo_change_image(PP_IMAGEINFO(ob->imageinfo));
	effectselector_set_current_list(EFFECTSELECTOR(ob->effectselector),ii);
	pp_imageinfo_refresh(PP_IMAGEINFO(ob->imageinfo));
	pp_histogram_refresh(PP_HISTOGRAM(ob->histogram));
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_ImageControl *ob=PP_IMAGECONTROL(userdata);
	ob->layout->state.SetInt("ExpanderState_EffectSelector",gtk_expander_get_expanded (GTK_EXPANDER(ob->expander1)));
}


GtkWidget*
pp_imagecontrol_new (Layout *layout)
{
	pp_ImageControl *ob=PP_IMAGECONTROL(g_object_new (pp_imagecontrol_get_type (), NULL));
	ob->layout=layout;

	GtkWidget *frame;

	// ImageInfo

	ob->imageinfo=pp_imageinfo_new(layout);
	gtk_box_pack_start(GTK_BOX(ob),ob->imageinfo,FALSE,FALSE,0);
	gtk_widget_show(ob->imageinfo);

	ob->histogram=pp_histogram_new(layout);
	gtk_box_pack_start(GTK_BOX(ob),ob->histogram,FALSE,FALSE,0);
	gtk_widget_show(ob->histogram);

	// FIXME add transformations here


	// FIXME add border selector here


	// EffectSelector

	ob->expander1=frame=gtk_expander_new(_("Effects"));
	g_signal_connect(ob->expander1, "notify::expanded",G_CALLBACK (expander_callback), ob);
	gtk_expander_set_expanded(GTK_EXPANDER(frame),layout->state.FindInt("ExpanderState_EffectSelector"));

	gtk_box_pack_start(GTK_BOX(ob),frame,TRUE,TRUE,0);
	gtk_widget_show(frame);

	ob->effectselector=effectselector_new();
	g_signal_connect(G_OBJECT(ob->effectselector),"changed",G_CALLBACK(effectselector_changed),ob);
	g_signal_connect(G_OBJECT(ob->effectselector),"addeffect",G_CALLBACK(effectselector_addeffect),ob);
	g_signal_connect(G_OBJECT(ob->effectselector),"removeeffect",G_CALLBACK(effectselector_removeeffect),ob);
	gtk_container_add(GTK_CONTAINER(frame),ob->effectselector);
	gtk_widget_show(ob->effectselector);

	return(GTK_WIDGET(ob));
}


GType
pp_imagecontrol_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_imagecontrol_info =
		{
			sizeof (pp_ImageControlClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_imagecontrol_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_ImageControl),
			0,
			(GInstanceInitFunc) pp_imagecontrol_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_VBOX, "pp_ImageControl", &pp_imagecontrol_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_imagecontrol_class_init (pp_ImageControlClass *klass)
{
	pp_imagecontrol_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_ImageControlClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_imagecontrol_init (pp_ImageControl *ob)
{
	ob->layout=NULL;
	ob->imageinfo=NULL;
	ob->effectselector=NULL;
	ob->expander1=NULL;
}

