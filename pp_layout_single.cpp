#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkexpander.h>

#include "config.h"

#include "stpui_widgets/stpui_combo.h"
#include "layout.h"
#include "dialogs.h"
#include "pp_pageextent.h"
#include "pp_imagecontrol.h"
#include "pp_layout_single_pageview.h"
#include "pp_layout_single.h"

#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	POPUPMENU_SIGNAL,
	SELECTIONCHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_layout_single_signals[LAST_SIGNAL] = { 0 };

static void pp_layout_single_class_init (pp_Layout_SingleClass *klass);
static void pp_layout_single_init (pp_Layout_Single *stpuicombo);


static void reflow(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	pp_Layout_Single_PageView *pv=PP_LAYOUT_SINGLE_PAGEVIEW(lo->pageview);
	Layout_Single *l=(Layout_Single*)lo->state->layout;
	l->Reflow();
	pp_layout_single_pageview_refresh(PP_LAYOUT_SINGLE_PAGEVIEW(pv));

	int pages=l->GetPages();
	gtk_widget_set_sensitive(lo->page,pages!=1);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(lo->page),1.0,pages);
	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void pe_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	pp_Layout_Single_PageView *pv=PP_LAYOUT_SINGLE_PAGEVIEW(lo->pageview);
	pp_layout_single_pageview_refresh(pv);
	g_signal_emit(G_OBJECT (ob),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void ic_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	pp_Layout_Single_PageView *pv=PP_LAYOUT_SINGLE_PAGEVIEW(lo->pageview);
	pp_layout_single_pageview_refresh(PP_LAYOUT_SINGLE_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void hscale_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;

	float hs=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lo->hscale));
	Layout_Single *l=(Layout_Single *)lo->state->layout;
	int page=l->GetCurrentPage();
	Layout_Single_ImageInfo *ii=l->ImageAt(page);

	if(ii)
	{
		if(hs>=10.0 && hs<=1000.0)
			ii->hscale=hs;
	}

	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void vscale_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;

	float vs=gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(lo->vscale));
	Layout_Single *l=(Layout_Single *)lo->state->layout;
	int page=l->GetCurrentPage();
	Layout_Single_ImageInfo *ii=l->ImageAt(page);

	if(ii)
	{
		if(vs>=10.0 && vs<=1000.0)
			ii->vscale=vs;	
	}

	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void page_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	pp_Layout_Single_PageView *pv=PP_LAYOUT_SINGLE_PAGEVIEW(lo->pageview);

	pp_layout_single_pageview_set_page(pv,gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->page))-1);
	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void pageview_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[CHANGED_SIGNAL], 0);
}


static void pageview_popupmenu(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[POPUPMENU_SIGNAL], 0);
}


static void pageview_selectionchanged(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Single *lo=(pp_Layout_Single *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_single_signals[SELECTIONCHANGED_SIGNAL], 0);
}


void pp_layout_single_refresh(pp_Layout_Single *ob)
{
	ob->state->layout->LayoutToDB(ob->state->layoutdb);

	int pages=ob->state->layout->GetPages();
	int currentpage=ob->state->layout->GetCurrentPage();

	gtk_widget_set_sensitive(ob->page,pages!=1);
	if(pages>1)
	{
		gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->page),1.0,pages);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->page),currentpage+1);
	}

	Layout_Single *l=(Layout_Single *)ob->state->layout;
	Layout_Single_ImageInfo *ii=l->ImageAt(currentpage);
	gtk_widget_set_sensitive(ob->hscale,ii!=0);
	gtk_widget_set_sensitive(ob->vscale,ii!=0);
	if(ii)
	{
		l->SelectNone();
		ii->SetSelected(true);
		pp_imagecontrol_set_image(PP_IMAGECONTROL(ob->imagecontrol));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->hscale),ii->hscale);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->vscale),ii->vscale);
	}
	
	pp_Layout_Single_PageView *pv=PP_LAYOUT_SINGLE_PAGEVIEW(ob->pageview);
	if(pv)
		pp_layout_single_pageview_refresh(pv);

	pp_imagecontrol_refresh(PP_IMAGECONTROL(ob->imagecontrol));

	pp_pageextent_refresh(PP_PAGEEXTENT(ob->pageextent));
}


void pp_layout_single_set_unit(GtkWidget *wid,enum Units unit)
{
	pp_Layout_Single *ob=PP_LAYOUT_SINGLE(wid);
	pp_pageextent_set_unit(PP_PAGEEXTENT(ob->pageextent),unit);
}


static void expander_callback(GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_Layout_Single *ob=PP_LAYOUT_SINGLE(userdata);
	ob->state->SetInt("ExpanderState_Single",gtk_expander_get_expanded (GTK_EXPANDER(object)));
}


GtkWidget*
pp_layout_single_new (PhotoPrint_State *state)
{
	pp_Layout_Single *ob=PP_LAYOUT_SINGLE(g_object_new (pp_layout_single_get_type (), NULL));

	gtk_container_set_border_width(GTK_CONTAINER(&ob->hbox),10);
	
	ob->state=state;
	
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;

	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX (&ob->hbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame); 
	
	ob->pageview = pp_layout_single_pageview_new ((Layout_Single *)ob->state->layout);
	g_signal_connect(G_OBJECT(ob->pageview),"changed",G_CALLBACK(pageview_changed),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"reflow",G_CALLBACK(reflow),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"popupmenu",G_CALLBACK(pageview_popupmenu),ob);
	g_signal_connect(G_OBJECT(ob->pageview),"selection_changed",G_CALLBACK(pageview_selectionchanged),ob);

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


	// Scale

	frame=gtk_expander_new(_("Manual scaling"));
	gtk_expander_set_expanded(GTK_EXPANDER(frame),state->FindInt("ExpanderState_Single"));
	g_signal_connect(frame, "notify::expanded",G_CALLBACK (expander_callback), ob);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	GtkWidget *table=gtk_table_new(4,2,false);
	gtk_table_set_row_spacing(GTK_TABLE(table),0,5);
	gtk_table_set_row_spacing(GTK_TABLE(table),2,3);
	gtk_table_set_col_spacings(GTK_TABLE(table),3);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_widget_show(table);


	//    HScale spin button

	label=gtk_label_new(_("H (%):"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	ob->hscale=gtk_spin_button_new_with_range(10.0,1000.0,0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->hscale),100);
	g_signal_connect(G_OBJECT(ob->hscale),"value-changed",G_CALLBACK(hscale_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),ob->hscale,1,2,0,1);
	gtk_widget_show(ob->hscale);

	
	//    VScale spin button

	label=gtk_label_new(_("V (%):"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	ob->vscale=gtk_spin_button_new_with_range(10.0,1000.0,0.1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->vscale),100);
	g_signal_connect(G_OBJECT(ob->vscale),"value-changed",G_CALLBACK(vscale_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),ob->vscale,1,2,1,2);
	gtk_widget_show(ob->vscale);


	// PageExtent

	ob->pageextent=pp_pageextent_new((Layout_Single *)ob->state->layout,ob->state);
	g_signal_connect(G_OBJECT(ob->pageextent),"changed",G_CALLBACK(pe_changed),ob);
	gtk_box_pack_start(GTK_BOX(vbox),ob->pageextent,FALSE,FALSE,0);
	gtk_widget_show(ob->pageextent);


	// ImageControl

	ob->imagecontrol=pp_imagecontrol_new(ob->state->layout);
	gtk_box_pack_start(GTK_BOX(vbox),ob->imagecontrol,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(ob->imagecontrol),"changed",G_CALLBACK(ic_changed),ob);
	gtk_widget_show(ob->imagecontrol);


	pp_layout_single_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_layout_single_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_layout_single_info =
		{
			sizeof (pp_Layout_SingleClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_layout_single_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Layout_Single),
			0,
			(GInstanceInitFunc) pp_layout_single_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "pp_Layout_Single", &pp_layout_single_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_layout_single_class_init (pp_Layout_SingleClass *klass)
{
	pp_layout_single_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_SingleClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_single_signals[POPUPMENU_SIGNAL] =
	g_signal_new ("popupmenu",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_SingleClass, popupmenu),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_single_signals[SELECTIONCHANGED_SIGNAL] =
	g_signal_new ("selection_changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_SingleClass, selection_changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_layout_single_init (pp_Layout_Single *ob)
{
	ob->state=NULL;
	ob->pageview=NULL;
	ob->imagecontrol=NULL;
}
