#include <iostream>
#include <gtk/gtk.h>

#include "ppeffect.h"
#include "effects_dialog.h"
#include "effectselector.h"
#include "effectlist.h"

#include "../support/debug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

using namespace std;

struct dialogstate
{
	PPEffectHeader *header;
	GtkWidget *preview;
	GdkPixbuf *previewsourcepb;
	GtkWidget *availselector;
	GtkWidget *currentselector;
};

#if 0
static void add_effect(GtkWidget *w,gpointer userdata)
{
	struct dialogstate *ds=(struct dialogstate *)userdata;
	EffectListItem *eli;
	if((eli=effectselector_get_selected(EFFECTSELECTOR(ds->availselector))))
	{
		Debug[TRACE] << "Got selection: " << eli->GetName() << endl;
//		eli->Action();
		// Add effect to PPEffectHeader here.
		effectselector_refresh(EFFECTSELECTOR(ds->currentselector));
	}
}


static void effect_settings(GtkWidget *w,gpointer userdata)
{
	struct dialogstate *ds=(struct dialogstate *)userdata;
	EffectListItem *eli;
	if((eli=effectselector_get_selected(EFFECTSELECTOR(ds->currentselector))))
	{
		Debug[TRACE] << "Got selection: " << eli->GetName() << endl;
//		eli->Action();
	}
}


static void remove_effect(GtkWidget *w,gpointer userdata)
{
	struct dialogstate *ds=(struct dialogstate *)userdata;
	EffectListItem *eli;
	if((eli=effectselector_get_selected(EFFECTSELECTOR(ds->currentselector))))
	{
		Debug[TRACE] << "Got selection: " << eli->GetName() << endl;
		eli->Remove();
		effectselector_refresh(EFFECTSELECTOR(ds->currentselector));
	}
}
#endif

void EffectsDialog(PPEffectHeader &header,GtkWindow *parent,GdkPixbuf *thumbnail)
{
	dialogstate st;
	st.header=&header;
	st.previewsourcepb=thumbnail;

	GtkWidget *dialog;

 	dialog=gtk_dialog_new_with_buttons(_("Effects..."),
		parent,GtkDialogFlags(0),
//		GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL,
		GTK_STOCK_OK,GTK_RESPONSE_OK,
		NULL);

	gtk_window_set_default_size(GTK_WINDOW(dialog),400,400);
	

	st.availselector=effectselector_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),st.availselector,TRUE,TRUE,0);
	gtk_widget_show(st.availselector);

	gtk_widget_show(dialog);

	effectselector_set_current_list(EFFECTSELECTOR(st.availselector),&header);
//	effectselector_set_current_list(EFFECTSELECTOR(st.availselector),NULL);
	
	bool done=false;
	while(!done)
	{
		gint result=gtk_dialog_run(GTK_DIALOG(dialog));
		switch(result)
		{
			case GTK_RESPONSE_CANCEL:
				done=true;
				break;
			case GTK_RESPONSE_OK:
				done=true;
				break;
		}
	}
	gtk_widget_destroy(dialog);
}

