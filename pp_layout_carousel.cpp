#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkexpander.h>

#include "config.h"

#include "stpui_widgets/dimension.h"
#include "stpui_widgets/stpui_combo.h"
#include "layout.h"
#include "dialogs.h"
#include "pp_pageextent.h"
#include "pp_imagecontrol.h"
#include "pp_layout_carousel_pageview.h"
#include "pp_layout_carousel.h"
#include "layout_carousel.h"

#include "debug.h"

#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	POPUPMENU_SIGNAL,
	SELECTIONCHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_layout_carousel_signals[LAST_SIGNAL] = { 0 };

static void pp_layout_carousel_class_init (pp_Layout_CarouselClass *klass);
static void pp_layout_carousel_init (pp_Layout_Carousel *stpuicombo);


static void reflow(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	pp_Layout_Carousel_PageView *pv=PP_LAYOUT_CAROUSEL_PAGEVIEW(lo->pageview);
	Layout_Carousel *l=(Layout_Carousel*)lo->state->layout;
	l->Reflow();
	pp_layout_carousel_pageview_refresh(PP_LAYOUT_CAROUSEL_PAGEVIEW(pv));

	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void pe_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	pp_Layout_Carousel_PageView *pv=PP_LAYOUT_CAROUSEL_PAGEVIEW(lo->pageview);
	pp_layout_carousel_pageview_refresh(pv);
	g_signal_emit(G_OBJECT (ob),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void ic_changed(GtkWidget *wid,gpointer *ob)
{
	Debug[TRACE] << "Got changed signal from ImageControl" << endl;
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	pp_Layout_Carousel_PageView *pv=PP_LAYOUT_CAROUSEL_PAGEVIEW(lo->pageview);
	pp_layout_carousel_pageview_refresh(PP_LAYOUT_CAROUSEL_PAGEVIEW(pv));
	g_signal_emit(G_OBJECT (ob),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void segments_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;

	int segs=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->segments));
	Layout_Carousel *l=(Layout_Carousel *)lo->state->layout;
	l->SetSegments(segs);

	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void overlap_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;

	int overlap=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->overlap));
	Layout_Carousel *l=(Layout_Carousel *)lo->state->layout;

	l->SetOverlap(overlap);

	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void innerradius_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;


	Dimension *d=DIMENSION(lo->innerradius);
	int ir=int(dimension_get_pt(d));
	
	Layout_Carousel *l=(Layout_Carousel *)lo->state->layout;
	l->SetInnerRadius(ir);

	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void angleoffset_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;

	int angleoffset=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->angleoffset));
	Layout_Carousel *l=(Layout_Carousel *)lo->state->layout;

	l->SetAngleOffset(angleoffset);

	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void page_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	pp_Layout_Carousel_PageView *pv=PP_LAYOUT_CAROUSEL_PAGEVIEW(lo->pageview);

	pp_layout_carousel_pageview_set_page(pv,gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lo->page))-1);
	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void pageview_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[CHANGED_SIGNAL], 0);
}


static void pageview_popupmenu(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[POPUPMENU_SIGNAL], 0);
}


static void pageview_selection_changed(GtkWidget *wid,gpointer *ob)
{
	pp_Layout_Carousel *lo=(pp_Layout_Carousel *)ob;
	pp_imagecontrol_refresh(PP_IMAGECONTROL(lo->imagecontrol));
	g_signal_emit(G_OBJECT (lo),pp_layout_carousel_signals[SELECTIONCHANGED_SIGNAL], 0);
}


void pp_layout_carousel_refresh(pp_Layout_Carousel *ob)
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

	Layout_Carousel *l=(Layout_Carousel *)ob->state->layout;

	int ms=l->CountImages(currentpage);
	ms=(ms+1)&~1;
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->segments),ms,16);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->segments),l->GetSegments());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->overlap),l->GetOverlap());

	dimension_set_pt(DIMENSION(ob->innerradius),l->GetInnerRadius());
	
	pp_Layout_Carousel_PageView *pv=PP_LAYOUT_CAROUSEL_PAGEVIEW(ob->pageview);
	if(pv)
		pp_layout_carousel_pageview_refresh(pv);

	pp_pageextent_refresh(PP_PAGEEXTENT(ob->pageextent));
}


void pp_layout_carousel_set_unit(GtkWidget *wid,enum Units unit)
{
	pp_Layout_Carousel *ob=PP_LAYOUT_CAROUSEL(wid);
	pp_pageextent_set_unit(PP_PAGEEXTENT(ob->pageextent),unit);
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_Layout_Carousel *ob=PP_LAYOUT_CAROUSEL(userdata);
	ob->state->SetInt("ExpanderState_Carousel",gtk_expander_get_expanded (GTK_EXPANDER(object)));
}


GtkWidget*
pp_layout_carousel_new (PhotoPrint_State *state)
{
	pp_Layout_Carousel *ob=PP_LAYOUT_CAROUSEL(g_object_new (pp_layout_carousel_get_type (), NULL));
	Layout_Carousel *l=(Layout_Carousel *)state->layout;

	gtk_container_set_border_width(GTK_CONTAINER(&ob->hbox),10);
	
	ob->state=state;
	
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *label;
	
	GtkWidget *frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX (&ob->hbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame); 
	
	ob->pageview = pp_layout_carousel_pageview_new ((Layout_Carousel *)ob->state->layout);
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


	// Carousel
	
	frame=gtk_expander_new(_("Carousel"));
	gtk_expander_set_expanded(GTK_EXPANDER(frame),state->FindInt("ExpanderState_Carousel"));
	g_signal_connect(frame, "notify::expanded",G_CALLBACK (expander_callback), ob);
	gtk_box_pack_start(GTK_BOX(vbox),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	GtkWidget *table=gtk_table_new(2,4,false);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);
//	gtk_table_set_row_spacing(GTK_TABLE(table),4,5);
	gtk_table_set_col_spacings(GTK_TABLE(table),3);
	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_widget_show(table);
	

	//    Segments spin button

	label=gtk_label_new(_("Segments:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,0,1);
	gtk_widget_show(label);

	ob->segments=gtk_spin_button_new_with_range(4.0,16.0,2.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->segments),l->GetSegments());
	g_signal_connect(G_OBJECT(ob->segments),"value-changed",G_CALLBACK(segments_changed),ob);
	gtk_widget_show(ob->segments);

	gtk_table_attach_defaults(GTK_TABLE(table),ob->segments,1,2,0,1);


	//    Overlap spin button

	label=gtk_label_new(_("Overlap (%):"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,1,2);
	gtk_widget_show(label);

	ob->overlap=gtk_spin_button_new_with_range(0.0,50.0,1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->overlap),l->GetOverlap());
	g_signal_connect(G_OBJECT(ob->overlap),"value-changed",G_CALLBACK(overlap_changed),ob);
	gtk_widget_show(ob->overlap);

	gtk_table_attach_defaults(GTK_TABLE(table),ob->overlap,1,2,1,2);


	//    AngleOffset spin button

	label=gtk_label_new(_("Starting angle:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,2,3);
	gtk_widget_show(label);

	ob->angleoffset=gtk_spin_button_new_with_range(0.0,90.0,1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->angleoffset),l->GetAngleOffset());
	g_signal_connect(G_OBJECT(ob->angleoffset),"value-changed",G_CALLBACK(angleoffset_changed),ob);
	gtk_widget_show(ob->angleoffset);

	gtk_table_attach_defaults(GTK_TABLE(table),ob->angleoffset,1,2,2,3);


	//    Inner radius dimension widget
	
	label=gtk_label_new(_("Inner Radius:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),label,0,1,3,4);
	gtk_widget_show(label);

	enum Units unit=state->GetUnits();

	ob->innerradius=dimension_new(0.0,100.0,unit);
//	g_signal_connect(G_OBJECT(ob->innerradius),"value-changed",G_CALLBACK(innerradius_changed),ob);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->overlap),l->GetOverlap());
	g_signal_connect(G_OBJECT(ob->innerradius),"value-changed",G_CALLBACK(innerradius_changed),ob);
	gtk_widget_show(ob->innerradius);

	gtk_table_attach_defaults(GTK_TABLE(table),ob->innerradius,1,2,3,4);


	// PageExtent

	ob->pageextent=pp_pageextent_new((Layout_Carousel *)ob->state->layout,ob->state);
	g_signal_connect(G_OBJECT(ob->pageextent),"changed",G_CALLBACK(pe_changed),ob);
	gtk_box_pack_start(GTK_BOX(vbox),ob->pageextent,FALSE,FALSE,0);
	gtk_widget_show(ob->pageextent);


	// ImageControl

	ob->imagecontrol=pp_imagecontrol_new(ob->state->layout);
	gtk_box_pack_start(GTK_BOX(vbox),ob->imagecontrol,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(ob->imagecontrol),"changed",G_CALLBACK(ic_changed),ob);
	gtk_widget_show(ob->imagecontrol);


#if 0
	// Spacer box

	tmp=gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(vbox),tmp,TRUE,TRUE,0);
	gtk_widget_show(tmp);


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
#endif

	pp_layout_carousel_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_layout_carousel_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_layout_carousel_info =
		{
			sizeof (pp_Layout_CarouselClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_layout_carousel_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Layout_Carousel),
			0,
			(GInstanceInitFunc) pp_layout_carousel_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "pp_Layout_Carousel", &pp_layout_carousel_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_layout_carousel_class_init (pp_Layout_CarouselClass *klass)
{
	pp_layout_carousel_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_CarouselClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_carousel_signals[POPUPMENU_SIGNAL] =
	g_signal_new ("popupmenu",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_CarouselClass, popupmenu),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_layout_carousel_signals[SELECTIONCHANGED_SIGNAL] =
	g_signal_new ("selection_changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_Layout_CarouselClass, selection_changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_layout_carousel_init (pp_Layout_Carousel *ob)
{
	ob->state=NULL;
	ob->pageview=NULL;
	ob->imagecontrol=NULL;
}
