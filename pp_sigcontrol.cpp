#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>

#include "stpui_widgets/dimension.h"
#include "signature.h"
#include "debug.h"
#include "miscwidgets/simplecombo.h"
#include "pp_pageextent.h"
#include "photoprint_state.h"

#include "pp_sigcontrol.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)
#define N_(x) gettext_noop(x)

enum {
	CHANGED_SIGNAL,
	REFLOW_SIGNAL,
	LAST_SIGNAL
};

static guint pp_sigcontrol_signals[LAST_SIGNAL] = { 0 };

static void pp_sigcontrol_class_init (pp_SigControlClass *klass);
static void pp_sigcontrol_init (pp_SigControl *stpuicombo);


static void refresh_gutters(pp_SigControl *ob)
{
	Debug[TRACE] << "Refreshing gutters" << endl;
	g_signal_handler_block(G_OBJECT(ob->hgutter),ob->hgutterhandler);
	g_signal_handler_block(G_OBJECT(ob->vgutter),ob->vgutterhandler);
	if(ob->hgutter)
		dimension_set_pt(DIMENSION(ob->hgutter),ob->sig->GetHGutter());
	if(ob->vgutter)
		dimension_set_pt(DIMENSION(ob->vgutter),ob->sig->GetVGutter());
	g_signal_handler_unblock(G_OBJECT(ob->hgutter),ob->hgutterhandler);
	g_signal_handler_unblock(G_OBJECT(ob->vgutter),ob->vgutterhandler);
}


static void refresh_size(pp_SigControl *ob)
{
	if(ob->rows)
	{
		g_signal_handler_block(G_OBJECT(ob->rows),ob->heighthandler);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->rows),ob->sig->GetRows());
		g_signal_handler_unblock(G_OBJECT(ob->rows),ob->heighthandler);
	}
	if(ob->cols)
	{
		g_signal_handler_block(G_OBJECT(ob->cols),ob->widthhandler);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->cols),ob->sig->GetColumns());
		g_signal_handler_unblock(G_OBJECT(ob->cols),ob->widthhandler);
	}

	refresh_gutters(ob);

	if(ob->width)
	{
		g_signal_handler_block(G_OBJECT(ob->width),ob->widthhandler);
		dimension_set_pt(DIMENSION(ob->width),ob->sig->GetCellWidth());
		g_signal_handler_unblock(G_OBJECT(ob->width),ob->widthhandler);
	}
	if(ob->height)
	{
		g_signal_handler_block(G_OBJECT(ob->height),ob->heighthandler);
		dimension_set_pt(DIMENSION(ob->height),ob->sig->GetCellHeight());
		g_signal_handler_unblock(G_OBJECT(ob->height),ob->heighthandler);
	}
}


void pp_sigcontrol_refresh(pp_SigControl *ob)
{
	if(ob->width)
		ob->sig->ReCalcByCellSize();

	refresh_size(ob);

	refresh_gutters(ob);
}


static void rows_changed(GtkWidget *wid,gpointer ob)
{
	Debug[TRACE] << "In rows_changed" << endl;
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	GtkSpinButton *spin=GTK_SPIN_BUTTON(wid);
	int v=gtk_spin_button_get_value_as_int(spin);
	Debug[TRACE] << "  got value: " << v << endl;
	sig->SetRows(v);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[REFLOW_SIGNAL], 0);
}


static void cols_changed(GtkWidget *wid,gpointer ob)
{
	Debug[TRACE] << "In cols_changed" << endl;
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	GtkSpinButton *spin=GTK_SPIN_BUTTON(wid);
	int v=gtk_spin_button_get_value_as_int(spin);
	Debug[TRACE] << "  got value: " << v << endl;
	sig->SetColumns(v);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[REFLOW_SIGNAL], 0);
}


static void hgutter_changed(GtkWidget *wid,gpointer ob)
{
	Debug[TRACE] << "In hgutter_changed" << endl;
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	int v=int(dimension_get_pt(DIMENSION(wid)));
	Debug[TRACE] << "  got value: " << v << endl;
	sig->SetHGutter(v);
	refresh_size(lo);
	refresh_gutters(lo);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
}


static void vgutter_changed(GtkWidget *wid,gpointer ob)
{
	Debug[TRACE] << "In vgutter_changed" << endl;
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	int v=int(dimension_get_pt(DIMENSION(wid)));
	Debug[TRACE] << "  got value: " << v << endl;
	sig->SetVGutter(v);
	refresh_size(lo);
	refresh_gutters(lo);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
}


static void width_changed(GtkWidget *wid,gpointer ob)
{
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	int v=int(dimension_get_pt(DIMENSION(wid)));
	sig->SetCellWidth(v);
	// Must check the newly set width and adjust the widget if different
	if(v!=sig->GetCellWidth())
		dimension_set_pt(DIMENSION(wid),sig->GetCellWidth());
	refresh_gutters(lo);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[REFLOW_SIGNAL], 0);
}


static void height_changed(GtkWidget *wid,gpointer ob)
{
	pp_SigControl *lo=(pp_SigControl *)ob;
	Signature *sig=lo->sig;
	int v=int(dimension_get_pt(DIMENSION(wid)));
	sig->SetCellHeight(v);
	// Must check the newly set width and adjust the widget if different
	if(v!=sig->GetCellHeight())
		dimension_set_pt(DIMENSION(wid),sig->GetCellHeight());
	refresh_gutters(lo);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
	g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[REFLOW_SIGNAL], 0);
}


static void getsizefromimage_clicked(GtkWidget *wid,gpointer ob)
{
	pp_SigControl *lo=(pp_SigControl *)ob;
	Layout_NUp *sig=lo->sig;
	LayoutIterator it(*sig);
	Layout_ImageInfo *ii=it.FirstSelected();
	if(ii)
	{
		int w=(72.0*ii->GetWidth())/ii->GetXRes();
		int h=(72.0*ii->GetHeight())/ii->GetYRes();
		Debug[TRACE] << "Natural size is " << w << " x " << h << endl;
		sig->SetCellWidth(w);
		sig->SetCellHeight(h);
		int t=sig->GetRows()*sig->GetColumns();
		sig->SetCellWidth(h);
		sig->SetCellHeight(w);
		if(t>(sig->GetRows()*sig->GetColumns()))
		{
			sig->SetCellWidth(w);
			sig->SetCellHeight(h);
		}
		pp_sigcontrol_refresh(lo);
		g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[CHANGED_SIGNAL], 0);
		g_signal_emit(G_OBJECT (ob),pp_sigcontrol_signals[REFLOW_SIGNAL], 0);
	}
}


void pp_sigcontrol_set_unit(pp_SigControl *ob,enum Units unit)
{
	ob->unit=unit;
	if(ob->hgutter)
		dimension_set_unit(DIMENSION(ob->hgutter),unit);
	if(ob->vgutter)
		dimension_set_unit(DIMENSION(ob->vgutter),unit);
	if(ob->width)
		dimension_set_unit(DIMENSION(ob->width),unit);
	if(ob->height)
		dimension_set_unit(DIMENSION(ob->height),unit);
}


static void build_rowscols(pp_SigControl *ob)
{
	ob->table=gtk_table_new(4,2,false);
	gtk_table_set_row_spacings(GTK_TABLE(ob->table),5);

	gtk_box_pack_start(GTK_BOX(ob->vbox),ob->table,FALSE,FALSE,0);

	//    Columns spin button
	
	GtkWidget *label=gtk_label_new(_("Columns:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,0,1);
	gtk_widget_show(label);

	ob->cols=gtk_spin_button_new_with_range(1.0,10.0,1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->cols),ob->sig->GetColumns());
	ob->widthhandler=g_signal_connect(G_OBJECT(ob->cols),"value-changed",G_CALLBACK(cols_changed),ob);
	gtk_widget_show(ob->cols);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->cols),1,2,0,1);

	//    Rows spin button

	label=gtk_label_new(_("Rows:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,1,2);
	gtk_widget_show(label);

	ob->rows=gtk_spin_button_new_with_range(1.0,10.0,1.0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->rows),ob->sig->GetRows());
	ob->heighthandler=g_signal_connect(G_OBJECT(ob->rows),"value-changed",G_CALLBACK(rows_changed),ob);
	gtk_widget_show(ob->rows);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->rows),1,2,1,2);

	//    HGutter spin button
	
	label=gtk_label_new(_("Gap:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),2,3,0,1);
	gtk_widget_show(label);

	ob->hgutter=dimension_new(0.0,200.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->hgutter),ob->sig->GetHGutter());
	ob->hgutterhandler=g_signal_connect(G_OBJECT(ob->hgutter),"value-changed",G_CALLBACK(hgutter_changed),ob);
	gtk_widget_show(ob->hgutter);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->hgutter),3,4,0,1);

	//    VGutter spin button

	label=gtk_label_new(_("Gap:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),2,3,1,2);
	gtk_widget_show(label);

	ob->vgutter=dimension_new(0.0,200.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->vgutter),ob->sig->GetVGutter());
	ob->vgutterhandler=g_signal_connect(G_OBJECT(ob->vgutter),"value-changed",G_CALLBACK(vgutter_changed),ob);
	gtk_widget_show(ob->vgutter);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->vgutter),3,4,1,2);

	gtk_widget_show(ob->table);
}


static void build_dimensions(pp_SigControl *ob)
{
	ob->table=gtk_table_new(4,2,false);
	gtk_table_set_row_spacings(GTK_TABLE(ob->table),5);

	gtk_box_pack_start(GTK_BOX(ob->vbox),ob->table,FALSE,FALSE,0);


	// Width
	GtkWidget *label=gtk_label_new(_("Width:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,0,1);
	gtk_widget_show(label);

	ob->width=dimension_new(0.0,10000.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->width),ob->sig->GetCellWidth());
	ob->widthhandler=g_signal_connect(G_OBJECT(ob->width),"value-changed",G_CALLBACK(width_changed),ob);
	gtk_widget_show(ob->width);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->width),1,2,0,1);


	// Height
	label=gtk_label_new(_("Height:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),0,1,1,2);
	gtk_widget_show(label);

	ob->height=dimension_new(0.0,10000.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->height),ob->sig->GetCellHeight());
	ob->heighthandler=g_signal_connect(G_OBJECT(ob->height),"value-changed",G_CALLBACK(height_changed),ob);
	gtk_widget_show(ob->height);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->height),1,2,1,2);

	GtkWidget *tmp=gtk_button_new_with_label(_("From selected image"));
	g_signal_connect(G_OBJECT(tmp),"clicked",G_CALLBACK(getsizefromimage_clicked),ob);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(tmp),0,4,2,3);
	gtk_widget_show(tmp);

	//    HGutter spin button
	
	label=gtk_label_new(_("Gap:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),2,3,0,1);
	gtk_widget_show(label);

	ob->hgutter=dimension_new(0.0,200.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->hgutter),ob->sig->GetHGutter());
	ob->hgutterhandler=g_signal_connect(G_OBJECT(ob->hgutter),"value-changed",G_CALLBACK(hgutter_changed),ob);
	gtk_widget_show(ob->hgutter);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->hgutter),3,4,0,1);

	//    VGutter spin button

	label=gtk_label_new(_("Gap:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(label),2,3,1,2);
	gtk_widget_show(label);

	ob->vgutter=dimension_new(0.0,200.0,ob->unit);
	dimension_set_pt(DIMENSION(ob->vgutter),ob->sig->GetVGutter());
	ob->vgutterhandler=g_signal_connect(G_OBJECT(ob->vgutter),"value-changed",G_CALLBACK(vgutter_changed),ob);
	gtk_widget_show(ob->vgutter);

	gtk_table_attach_defaults(GTK_TABLE(ob->table),GTK_WIDGET(ob->vgutter),3,4,1,2);

	gtk_widget_show(ob->table);
}


// Combo changed will clean up any existing widgets and build
// a new table of widgets suitable for the selected mode.

static void combo_changed(GtkWidget *wid,gpointer userdata)
{
	pp_SigControl *ob=(pp_SigControl *)userdata;
	if(ob->table)
		gtk_widget_destroy(ob->table);
	ob->table=NULL;
	ob->rows=NULL;
	ob->cols=NULL;
	ob->hgutter=NULL;
	ob->vgutter=NULL;
	const char *sel=simplecombo_get(SIMPLECOMBO(ob->combo));
	if(strcmp(sel,"RowsCols")==0)
	{
		build_rowscols(ob);
	}
	else if(strcmp(sel,"Dimensions")==0)
	{
		build_dimensions(ob);
	}
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_SigControl *ob=PP_SIGCONTROL(object);
	ob->sig->state.SetInt("ExpanderState_SigControl",gtk_expander_get_expanded (GTK_EXPANDER(ob)));
}


GtkWidget*
pp_sigcontrol_new (Layout_NUp *sig,enum Units unit)
{
	pp_SigControl *ob=PP_SIGCONTROL(g_object_new (pp_sigcontrol_get_type (), NULL));
//	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->sig=sig;
	ob->unit=unit;
	
	ob->vbox=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(ob),ob->vbox);
	gtk_widget_show(ob->vbox);

	// Layout frame

	gtk_expander_set_expanded(GTK_EXPANDER(ob),sig->state.FindInt("ExpanderState_SigControl"));
	g_signal_connect(ob, "notify::expanded",G_CALLBACK (expander_callback), NULL);
	gtk_expander_set_label(GTK_EXPANDER(ob),_("Layout"));

	SimpleComboOptions comboopts;
	comboopts.Add("RowsCols",_("Specify rows/columns"));
	comboopts.Add("Dimensions",_("Specify image dimensions"));

	ob->combo=simplecombo_new(comboopts);
	if(sig->GetAbsolute())
		simplecombo_set(SIMPLECOMBO(ob->combo),"Dimensions");
	gtk_box_pack_start(GTK_BOX(ob->vbox),ob->combo,FALSE,FALSE,8);
	g_signal_connect(G_OBJECT(ob->combo),"changed",G_CALLBACK(combo_changed),ob);
	gtk_widget_show(ob->combo);

	combo_changed(ob->combo,ob);

	pp_sigcontrol_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_sigcontrol_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_sigcontrol_info =
		{
			sizeof (pp_SigControlClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_sigcontrol_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_SigControl),
			0,
			(GInstanceInitFunc) pp_sigcontrol_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_EXPANDER, "pp_SigControl", &pp_sigcontrol_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_sigcontrol_class_init (pp_SigControlClass *klass)
{
	pp_sigcontrol_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_SigControlClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	pp_sigcontrol_signals[REFLOW_SIGNAL] =
	g_signal_new ("reflow",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_SigControlClass, reflow),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_sigcontrol_init (pp_SigControl *ob)
{
	ob->sig=NULL;
}


int pp_sigcontrol_get_expander_state(pp_SigControl *ob)
{
	int result=0;
	if(gtk_expander_get_expanded(GTK_EXPANDER(ob)))
		result=1;
	return(result);
}


void pp_sigcontrol_set_expander_state(pp_SigControl *ob,int state)
{
	gtk_expander_set_expanded(GTK_EXPANDER(ob),state!=0);
}

