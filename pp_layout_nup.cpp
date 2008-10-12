#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtkhseparator.h>

#include "config.h"

#include "stpui_widgets/stpui_combo.h"

#include "pp_imagecontrol.h"

#include "effects/ppeffect.h"
#include "effects/effectselector.h"
#include "pp_imageinfo.h"

#include "pp_sigcontrol.h"
#include "pp_pageextent.h"
#include "photoprint_state.h"

#include "pp_layout_nup.h"

#include "support/tiffsave.h"
#include "layout.h"
#include "dialogs.h"

#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	POPUPMENU_SIGNAL,
	SELECTIONCHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_layout_nup_signals[LAST_SIGNAL] = { 0 };

static void pp_layout_nup_class_init (pp_Layout_NUpClass *klass);
static void pp_layout_nup_init (pp_Layout_NUp *stpuicombo);


static void sig_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(lo->pageview);
	Layout_NUp *l=(Layout_NUp*)lo->state->layout;
	l->FlushHRPreviews();
	pp_layout_nup_pageview_refresh(PP_LAYOUT_NUP_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void pe_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	Layout_NUp *l=(Layout_NUp*)lo->state->layout;
	l->ReCalc();
	l->FlushHRPreviews();
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(lo->pageview);
	pp_layout_nup_pageview_refresh(PP_LAYOUT_NUP_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void ic_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(lo->pageview);
	Layout_NUp *l=(Layout_NUp*)lo->state->layout;
	cerr << "Flushing due to ic_changed" << endl;

	// Loop through twice for performance reasons - firstly to cancel the
	// thread, secondly to wait for thread exit, and delete the preview.
	Layout_ImageInfo *ii=l->FirstSelected();
	{
		ii->CancelRenderThread();
		ii=l->NextSelected();
	}
	ii=l->FirstSelected();
	{
		ii->FlushHRPreview();
		ii=l->NextSelected();
	}
		
	pp_layout_nup_pageview_refresh(PP_LAYOUT_NUP_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void reflow(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
//	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(lo->pageview);
	Layout_NUp *l=(Layout_NUp*)lo->state->layout;
	l->CancelRenderThreads();
	l->Reflow();
	g_signal_emit(G_OBJECT (lo),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void page_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(lo->pageview);
	Layout_NUp *l=(Layout_NUp*)lo->state->layout;
	l->CancelRenderThreads();

	pp_layout_nup_pageview_set_page(pv,gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->page))-1);
	g_signal_emit(G_OBJECT (lo),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void pageview_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_nup_signals[CHANGED_SIGNAL], 0);
}


static void pageview_popupmenu(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_nup_signals[POPUPMENU_SIGNAL], 0);
}


static void pageview_selection_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_NUp *lo=(pp_Layout_NUp *)ob;
	pp_imagecontrol_set_image(PP_IMAGECONTROL(lo->imagecontrol));
	g_signal_emit(G_OBJECT (lo),pp_layout_nup_signals[SELECTIONCHANGED_SIGNAL], 0);
}


void pp_layout_nup_refresh(pp_Layout_NUp *ob)
{
	pp_Layout_NUp_PageView *pv=PP_LAYOUT_NUP_PAGEVIEW(ob->pageview);

	ob->state->layout->LayoutToDB(ob->state->layoutdb);

	int pages=ob->state->layout->GetPages();
	gtk_widget_set_sensitive(ob->page,pages!=1);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->page),1.0,pages);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->page),ob->state->layout->GetCurrentPage()+1);
	pp_layout_nup_pageview_refresh(pv);
	pp_sigcontrol_refresh(PP_SIGCONTROL(ob->sigcontrol));
	pp_pageextent_refresh(PP_PAGEEXTENT(ob->pageextent));
	pp_imagecontrol_refresh(PP_IMAGECONTROL(ob->imagecontrol));
}


void pp_layout_nup_set_unit(GtkWidget *wid,enum Units unit)
{
	pp_Layout_NUp *ob=PP_LAYOUT_NUP(wid);
	pp_pageextent_set_unit(PP_PAGEEXTENT(ob->pageextent),unit);
	pp_sigcontrol_set_unit(PP_SIGCONTROL(ob->sigcontrol),unit);
}


#if 0
static void killshadow(GtkWidget *wid,gpointer data)
{
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(wid),GTK_SHADOW_NONE);
}
#endif


GtkWidget*
pp_layout_nup_new (PhotoPrint_State *state)
{
	pp_Layout_NUp *ob=PP_LAYOUT_NUP(g_object_new (pp_layout_nup_get_type (), NULL));

	gtk_container_set_border_width(GTK_CONTAINER(&ob->hbox),10);
	
	ob->state=state;
	
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;

	enum Units unit=state->GetUnits();

	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX (&ob->hbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
	
	ob->pageview = pp_layout_nup_pageview_new ((Layout_NUp *)ob->state->layout);
	g_signal_connect(G_OBJECT(ob->pageview),"changed",G_CALLBACK(pageview_changed),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"reflow",G_CALLBACK(reflow),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"popupmenu",G_CALLBACK(pageview_popupmenu),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"selection_changed",G_CALLBACK(pageview_selection_changed),ob);

	gtk_container_add (GTK_CONTAINER (frame), ob->pageview);
	gtk_widget_show (ob->pageview);


	// Scroll Window

	GtkWidget *scrollwin=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin),
                                    GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	vbox = gtk_vbox_new (FALSE, 5);
	gtk_box_pack_start(GTK_BOX(&ob->hbox),vbox,FALSE,FALSE,0);
	gtk_widget_show (vbox);
	gtk_box_pack_start(GTK_BOX(vbox),scrollwin,TRUE,TRUE,0);
	gtk_widget_show (scrollwin);


	// Page number

	hbox=gtk_hbox_new(FALSE,5);
	label=gtk_label_new(_("Page:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
	gtk_widget_show(label);

	ob->page=gtk_spin_button_new_with_range(1.0,2.0,1.0);
	g_signal_connect(G_OBJECT(ob->page),"value-changed",G_CALLBACK(page_changed),ob);
	gtk_widget_show(ob->page);

	gtk_box_pack_start(GTK_BOX(hbox),ob->page,FALSE,FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
	gtk_widget_show(hbox);


	// Contents of scrollwindow

	GtkWidget *hbox2=gtk_hbox_new(FALSE,0);
	gtk_widget_show (hbox2);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox2),vbox,TRUE,TRUE,5);
	
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollwin), hbox2);
//	gtk_container_foreach(GTK_CONTAINER(scrollwin),killshadow,NULL);
	gtk_widget_show (vbox);


	// SigControl

	ob->sigcontrol=pp_sigcontrol_new((Layout_NUp *)ob->state->layout,unit);
	gtk_box_pack_start(GTK_BOX(vbox),ob->sigcontrol,FALSE,FALSE,0);
	g_signal_connect(G_OBJECT(ob->sigcontrol),"changed",G_CALLBACK(sig_changed),ob);
	g_signal_connect(G_OBJECT(ob->sigcontrol),"reflow",G_CALLBACK(reflow),ob);
	gtk_widget_show(ob->sigcontrol);


	// PageExtent

	ob->pageextent=pp_pageextent_new((Layout_NUp *)ob->state->layout,ob->state);
	g_signal_connect(G_OBJECT(ob->pageextent),"changed",G_CALLBACK(pe_changed),ob);
	gtk_box_pack_start(GTK_BOX(vbox),ob->pageextent,FALSE,FALSE,0);
	gtk_widget_show(ob->pageextent);


	// ImageControl

	ob->imagecontrol=pp_imagecontrol_new(ob->state->layout);
	gtk_box_pack_start(GTK_BOX(vbox),ob->imagecontrol,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(ob->imagecontrol),"changed",G_CALLBACK(ic_changed),ob);
	gtk_widget_show(ob->imagecontrol);

//  Need to store and retrieve this state information.
//	pp_imagecontrol_set_expander_state(PP_IMAGECONTROL(ob->imagecontrol),0);	

	pp_layout_nup_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_layout_nup_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_layout_nup_info =
		{
			sizeof (pp_Layout_NUpClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_layout_nup_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Layout_NUp),
			0,
			(GInstanceInitFunc) pp_layout_nup_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "pp_Layout_NUp", &pp_layout_nup_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_layout_nup_class_init (pp_Layout_NUpClass *klass)
{
	pp_layout_nup_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUpClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_nup_signals[SELECTIONCHANGED_SIGNAL] =
	g_signal_new ("selection_changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUpClass, popupmenu),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_nup_signals[POPUPMENU_SIGNAL] =
	g_signal_new ("popupmenu",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_NUpClass, popupmenu),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_layout_nup_init (pp_Layout_NUp *ob)
{
	ob->state=NULL;
	ob->pageview=NULL;
	ob->sigcontrol=NULL;
	ob->pageextent=NULL;
	ob->imagecontrol=NULL;
	ob->page=NULL;
}
