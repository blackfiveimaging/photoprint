/*
 * pp_pageextent.cpp - provides a custom widget for controlling a PageExtent.
 * Uses the GPrinter from PhotoPrint_State to display a list of pagesizes
 * and to get the page size.  The margins are applied independently to the
 * supplied PageExtent.
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkexpander.h>

#include "stpui_widgets/dimension.h"
#include "stpui_widgets/stpui_combo.h"

#include "photoprint_state.h"
#include "pp_pageextent.h"

#include "support/debug.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint pp_pageextent_signals[LAST_SIGNAL] = { 0 };

static void pp_pageextent_class_init (pp_PageExtentClass *klass);
static void pp_pageextent_init (pp_PageExtent *stpuicombo);


static void customwidth_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	if(lo->blocksignals)
		return;
	Dimension *d=DIMENSION(lo->customwidth);
	int v=int(dimension_get_pt(d));
	Debug[TRACE] << "Setting custom width to " << v << endl;
	lo->state->printer.SetCustomWidth(v);
	lo->state->layout->UpdatePageSize();
	pp_pageextent_refresh(lo);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void customheight_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	if(lo->blocksignals)
		return;
	Dimension *d=DIMENSION(lo->customheight);
	int v=int(dimension_get_pt(d));
	Debug[TRACE] << "Setting custom height to " << v << endl;
	lo->state->printer.SetCustomHeight(v);
	lo->state->layout->UpdatePageSize();
	pp_pageextent_refresh(lo);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void lmargin_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	PageExtent *pe=lo->pe;
	Dimension *d=DIMENSION(lo->lmargin);
	int v=int(dimension_get_pt(d));
	pe->SetMargins(v,pe->rightmargin,pe->topmargin,pe->bottommargin);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void rmargin_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	PageExtent *pe=lo->pe;
	Dimension *d=DIMENSION(lo->rmargin);
	int v=int(dimension_get_pt(d));
	pe->SetMargins(pe->leftmargin,v,pe->topmargin,pe->bottommargin);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void tmargin_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	PageExtent *pe=lo->pe;
	Dimension *d=DIMENSION(lo->tmargin);
	int v=int(dimension_get_pt(d));
	pe->SetMargins(pe->leftmargin,pe->rightmargin,v,pe->bottommargin);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void bmargin_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;
	PageExtent *pe=lo->pe;
	Dimension *d=DIMENSION(lo->bmargin);
	int v=int(dimension_get_pt(d));
	pe->SetMargins(pe->leftmargin,pe->rightmargin,pe->topmargin,v);
	g_signal_emit(G_OBJECT (ob),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


static void setcustomsizewidgets(pp_PageExtent *lo)
{
	lo->blocksignals=true;
	GPrinter *p=&lo->state->printer;
	int nw=0,mw=0,nh=0,mh=0;
	Debug[TRACE] << "Getting size limits..." << endl;
	p->GetSizeLimits(nw,mw,nh,mh);
	Debug[TRACE] << "Comparing min and max width:" << endl;
	if(nw==mw)
	{
		Debug[TRACE] << "No width adjustment possible..." << endl;
		gtk_widget_hide(lo->customwidth);
		gtk_widget_hide(lo->customwidthlabel);
	}
	else
	{
		Debug[TRACE] << "Allowing width adjustment..." << endl;
		gtk_widget_show(lo->customwidth);
		gtk_widget_show(lo->customwidthlabel);
		Debug[TRACE] << "Setting range to :" << nw << " -> " << mw << endl;
		dimension_set_range_pt(DIMENSION(lo->customwidth),nw,mw);
		dimension_set_pt(DIMENSION(lo->customwidth),p->pagewidth);
	}

	Debug[TRACE] << "Comparing min and max height:" << endl;
	if(nh==mh)
	{
		Debug[TRACE] << "No height adjustment possible..." << endl;
		gtk_widget_hide(lo->customheight);
		gtk_widget_hide(lo->customheightlabel);
	}
	else
	{
		Debug[TRACE] << "Allowing height adjustment..." << endl;
		gtk_widget_show(lo->customheight);
		gtk_widget_show(lo->customheightlabel);
		Debug[TRACE] << "Setting range to :" << nh << " -> " << mh << endl;
		dimension_set_range_pt(DIMENSION(lo->customheight),nh,mh);
		dimension_set_pt(DIMENSION(lo->customheight),p->pageheight);
	}
	lo->blocksignals=false;
}


static void pagesize_changed(GtkWidget *wid,gpointer *ob)
{
	pp_PageExtent *lo=(pp_PageExtent *)ob;

	lo->state->layout->UpdatePageSize();

	setcustomsizewidgets(lo);

	pp_pageextent_refresh(lo);
	g_signal_emit(G_OBJECT (lo),pp_pageextent_signals[CHANGED_SIGNAL], 0);
}


void pp_pageextent_refresh(pp_PageExtent *ob)
{
	ob->blocksignals=true;
	dimension_set_pt(DIMENSION(ob->lmargin),ob->pe->leftmargin);
	dimension_set_pt(DIMENSION(ob->rmargin),ob->pe->rightmargin);
	dimension_set_pt(DIMENSION(ob->tmargin),ob->pe->topmargin);
	dimension_set_pt(DIMENSION(ob->bmargin),ob->pe->bottommargin);
	dimension_set_pt(DIMENSION(ob->customwidth),ob->pe->pagewidth);
	dimension_set_pt(DIMENSION(ob->customheight),ob->pe->pageheight);

	stpui_combo_refresh(STPUI_COMBO(ob->pagesize));
	ob->blocksignals=false;
}


void pp_pageextent_set_unit(pp_PageExtent *ob,enum Units unit)
{
	dimension_set_unit(DIMENSION(ob->lmargin),unit);
	dimension_set_unit(DIMENSION(ob->rmargin),unit);
	dimension_set_unit(DIMENSION(ob->tmargin),unit);
	dimension_set_unit(DIMENSION(ob->bmargin),unit);
	dimension_set_unit(DIMENSION(ob->customwidth),unit);
	dimension_set_unit(DIMENSION(ob->customheight),unit);
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_PageExtent *ob=PP_PAGEEXTENT(object);
	ob->state->SetInt("ExpanderState_PageExtent",gtk_expander_get_expanded (GTK_EXPANDER(ob)));
}


GtkWidget*
pp_pageextent_new (PageExtent *pe,PhotoPrint_State *state)
{
	pp_PageExtent *ob=PP_PAGEEXTENT(g_object_new (pp_pageextent_get_type (), NULL));
//	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->pe=pe;
	ob->state=state;
	
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *vbox;

	enum Units unit=state->GetUnits();


	// PageSize Selector

//	frame=gtk_expander_new(_("Page Size and Margins"));
	gtk_expander_set_expanded(GTK_EXPANDER(ob),state->FindInt("ExpanderState_PageExtent"));
	g_signal_connect(ob, "notify::expanded",G_CALLBACK (expander_callback), NULL);

	gtk_expander_set_label(GTK_EXPANDER(ob),_("Page Size and Margins"));
//	gtk_box_pack_start(GTK_BOX(ob),frame,FALSE,FALSE,0);
//	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);

	GtkWidget *table=gtk_table_new(4,4,false);
	gtk_table_set_row_spacings(GTK_TABLE(table),5);

	gtk_container_add(GTK_CONTAINER(ob),vbox);
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,8);

	ob->pagesize=stpui_combo_new(ob->state->printer.stpvars,"PageSize",NULL);
	g_signal_connect(G_OBJECT(ob->pagesize),"changed",G_CALLBACK(pagesize_changed),ob);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->pagesize),0,4,0,1);
	gtk_widget_show(ob->pagesize);


	// Hack around Gutenprint locale problems
	setlocale(LC_ALL,"");

	// Custom page size

	ob->customwidthlabel=gtk_label_new(_("W:"));
	gtk_misc_set_alignment(GTK_MISC(ob->customwidthlabel),1.0,0.5);
	gtk_widget_show(ob->customwidthlabel);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->customwidthlabel),0,1,1,2);

	ob->customwidth=dimension_new(0.0,10.0,unit);
	gtk_widget_show(ob->customwidth);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->customwidth),1,2,1,2);


	ob->customheightlabel=gtk_label_new(_("H:"));
	gtk_widget_show(ob->customheightlabel);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->customheightlabel),2,3,1,2);
	gtk_misc_set_alignment(GTK_MISC(ob->customheightlabel),1.0,0.5);

	ob->customheight=dimension_new(0.0,10.0,unit);
	gtk_widget_show(ob->customheight);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->customheight),3,4,1,2);


	//    LeftMargin spin button

	
	label=gtk_label_new(_("Left:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),0,1,2,3);
	gtk_widget_show(label);

	ob->lmargin=dimension_new(0.0,600.0,unit);
	g_signal_connect(G_OBJECT(ob->lmargin),"value-changed",G_CALLBACK(lmargin_changed),ob);
	gtk_widget_show(ob->lmargin);
	
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->lmargin),1,2,2,3);


	//    RightMargin spin button

	hbox=gtk_hbox_new(FALSE,0);
	
	label=gtk_label_new(_("Right:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),2,3,2,3);
	gtk_widget_show(label);

	ob->rmargin=dimension_new(0.0,600.0,unit);
	g_signal_connect(G_OBJECT(ob->rmargin),"value-changed",G_CALLBACK(rmargin_changed),ob);
	gtk_widget_show(ob->rmargin);

	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->rmargin),3,4,2,3);

	//    TopMargin spin button

	label=gtk_label_new(_("Top:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),0,1,3,4);
	gtk_widget_show(label);

	ob->tmargin=dimension_new(0.0,800.0,unit);
	g_signal_connect(G_OBJECT(ob->tmargin),"value-changed",G_CALLBACK(tmargin_changed),ob);
	gtk_widget_show(ob->tmargin);

	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->tmargin),1,2,3,4);
	
	label=gtk_label_new(_("Bottom:"));
	gtk_misc_set_alignment(GTK_MISC(label),1.0,0.5);
	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(label),2,3,3,4);
	gtk_widget_show(label);

	ob->bmargin=dimension_new(0.0,800.0,unit);
	g_signal_connect(G_OBJECT(ob->bmargin),"value-changed",G_CALLBACK(bmargin_changed),ob);
	gtk_widget_show(ob->bmargin);

	gtk_table_attach_defaults(GTK_TABLE(table),GTK_WIDGET(ob->bmargin),3,4,3,4);

	gtk_widget_show(table);

	ob->state->layout->UpdatePageSize();
	setcustomsizewidgets(ob);

	g_signal_connect(G_OBJECT(ob->customheight),"value-changed",G_CALLBACK(customheight_changed),ob);
	g_signal_connect(G_OBJECT(ob->customwidth),"value-changed",G_CALLBACK(customwidth_changed),ob);

	pp_pageextent_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_pageextent_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_pageextent_info =
		{
			sizeof (pp_PageExtentClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_pageextent_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_PageExtent),
			0,
			(GInstanceInitFunc) pp_pageextent_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_EXPANDER, "pp_PageExtent", &pp_pageextent_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_pageextent_class_init (pp_PageExtentClass *klass)
{
	pp_pageextent_signals[CHANGED_SIGNAL] =
	g_signal_new ("changed",
		G_TYPE_FROM_CLASS (klass),
		GSignalFlags(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
		G_STRUCT_OFFSET (pp_PageExtentClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
pp_pageextent_init (pp_PageExtent *ob)
{
	ob->pe=NULL;
	ob->blocksignals=false;
}


int pp_pageextent_get_expander_state(pp_PageExtent *ob)
{
	int result=0;
	if(gtk_expander_get_expanded(GTK_EXPANDER(ob)))
		result=1;
	return(result);
}


void pp_pageextent_set_expander_state(pp_PageExtent *ob,int state)
{
	gtk_expander_set_expanded(GTK_EXPANDER(ob),state!=0);
}

