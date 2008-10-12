/*
 * dimension.cpp - provides a custom widget for adjusting
 * a physical dimension
 * Copyright (c) 2004 by Alastair M. Robinson
 * Distributed under the terms of the GNU General Public License -
 * see the file named "COPYING" for more details.
 *
 */

#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>

#include <math.h>
#include "dimension.h"

enum {
	CHANGED_SIGNAL,
	LAST_SIGNAL
};

static guint dimension_signals[LAST_SIGNAL] = { 0 };

static void dimension_class_init (DimensionClass *klass);
static void dimension_init (Dimension *stpuicombo);


static void spin_changed(GtkWidget *wid,gpointer *ob)
{
	Dimension *lo=(Dimension *)ob;
//	GtkSpinButton *spin=GTK_SPIN_BUTTON(lo->spinbutton);

	g_signal_handlers_block_matched (G_OBJECT(lo->spinbutton),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, lo);

	g_signal_emit(G_OBJECT (ob),dimension_signals[CHANGED_SIGNAL], 0);

	g_signal_handlers_unblock_matched (G_OBJECT(lo->spinbutton),G_SIGNAL_MATCH_DATA,
		0, 0, NULL, NULL, lo);
}


void dimension_refresh(Dimension *ob)
{

}


GtkWidget* dimension_new (gdouble min,gdouble max,enum Units unit)
{
	Dimension *ob=DIMENSION(g_object_new (dimension_get_type (), NULL));
	gtk_box_set_spacing(GTK_BOX(ob),5);

	ob->value=min;
	ob->minpt=min;
	ob->maxpt=max;
	ob->unit=UNIT_POINTS;

	ob->spinbutton=gtk_spin_button_new_with_range(min,max,0.01);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->spinbutton),ob->value);
	gtk_box_pack_start(GTK_BOX(ob),ob->spinbutton,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(ob->spinbutton),"value-changed",G_CALLBACK(spin_changed),ob);
	gtk_widget_show(ob->spinbutton);

	ob->label=gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(ob),ob->label,TRUE,TRUE,0);
//	gtk_widget_show(ob->label);

	ob->tooltips=gtk_tooltips_new();

	dimension_set_unit(ob,unit);
	dimension_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
dimension_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo dimension_info =
		{
			sizeof (DimensionClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) dimension_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (Dimension),
			0,
			(GInstanceInitFunc) dimension_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_HBOX, "Dimension", &dimension_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
dimension_class_init (DimensionClass *klass)
{
	dimension_signals[CHANGED_SIGNAL] =
	g_signal_new ("value-changed",
		G_TYPE_FROM_CLASS (klass),
		G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
		G_STRUCT_OFFSET (DimensionClass, changed),
		NULL, NULL,
		g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
dimension_init (Dimension *ob)
{
	ob->spinbutton=NULL;
	ob->label=NULL;
}


int dimension_get_pt(Dimension *ob)
{
	double v=gtk_spin_button_get_value(GTK_SPIN_BUTTON(ob->spinbutton));
	switch(ob->unit)
	{
		case UNIT_POINTS:
			break;
		case UNIT_INCHES:
			v=UNIT_ROUND_INCHES_TO_POINTS(v);
			break;
		case UNIT_MILLIMETERS:
			v=UNIT_ROUND_MILLIMETERS_TO_POINTS(v);
			break;
		case UNIT_CENTIMETERS:
			v=UNIT_ROUND_CENTIMETERS_TO_POINTS(v);
			break;
	}
	ob->value=v;
	return(v);
}


void dimension_update_unit_label(Dimension *ob)
{
	char *txt=NULL;
	char *tooltip=NULL;
	switch(ob->unit)
	{
		case UNIT_POINTS:
			txt="pt";
			tooltip=_("points");
			break;
		case UNIT_INCHES:
			txt="in";
			tooltip=_("inches");
			break;
		case UNIT_MILLIMETERS:
			txt="mm";
			tooltip=_("millimeters");
			break;
		case UNIT_CENTIMETERS:
			txt="cm";
			tooltip=_("centimeters");
			break;
	}
	if(txt)
		gtk_label_set_text(GTK_LABEL(ob->label),txt);
	if(tooltip)
		gtk_tooltips_set_tip(GTK_TOOLTIPS(ob->tooltips),GTK_WIDGET(ob->spinbutton),tooltip,NULL);
}


double mmtest[]={0.0,1.0,5.5,10.5};
double intest[]={0.0,1.0,5.5,10.5};
int pttest[]={0,1,50,72,75,100};

void sanitycheck()
{
	int i;
	double r;

	for(i=0;i<(sizeof(mmtest)/sizeof(float));++i)
	{
		r=UNIT_MILLIMETERS_TO_POINTS(mmtest[i]);
		printf("%lfmm -> %lfpt ",mmtest[i],r);
		printf("(%lfmm)\n",ROUNDTONEAREST(r,POINTS_PRECISION));
	}
	printf("\n");
	for(i=0;i<(sizeof(intest)/sizeof(float));++i)
	{
		r=UNIT_INCHES_TO_POINTS(intest[i]);
		printf("%lfin -> %lfpt ",intest[i],r);
		printf("(%lfin)\n",ROUNDTONEAREST(r,INCHES_PRECISION));
	}
	printf("\n");
	for(i=0;i<(sizeof(pttest)/sizeof(int));++i)
	{
		r=UNIT_POINTS_TO_MILLIMETERS(pttest[i]);
		printf("%dpt -> %lfmm ",pttest[i],r);
		printf("(%lfin)\n",ROUNDTONEAREST(r,MILLIMETERS_PRECISION));
	}
}


void dimension_set_unit(Dimension *ob,enum Units unit)
{
	gdouble v=gtk_spin_button_get_value(GTK_SPIN_BUTTON(ob->spinbutton));
	gdouble min=-1,max=-1,step=-1;
//	fprintf(stderr,"Initial value (unit %d): %f\n",ob->unit,v);
//	fprintf(stderr,"Initial range (pts): %f -> %f\n",ob->minpt,ob->maxpt);
	switch(ob->unit)
	{
		case UNIT_POINTS:
			break;
		case UNIT_INCHES:
			v=UNIT_ROUND_INCHES_TO_POINTS(v);
			break;
		case UNIT_MILLIMETERS:
			v=UNIT_ROUND_MILLIMETERS_TO_POINTS(v);
			break;
		case UNIT_CENTIMETERS:
			v=UNIT_ROUND_CENTIMETERS_TO_POINTS(v);
			break;
		default:
			fprintf(stderr,"PANIC! Unit %d was not caught!\n",ob->unit);
			break;
	}
	ob->unit=unit;
	switch(ob->unit)
	{
		case UNIT_POINTS:
			min=ob->minpt;
			max=ob->maxpt;
			step=POINTS_PRECISION;
			break;
		case UNIT_INCHES:
			v=UNIT_ROUND_POINTS_TO_INCHES(v);
			min=UNIT_ROUND_POINTS_TO_INCHES(ob->minpt);
			max=UNIT_ROUND_POINTS_TO_INCHES(ob->maxpt);
			step=INCHES_PRECISION;
			break;
		case UNIT_MILLIMETERS:
			v=UNIT_ROUND_POINTS_TO_MILLIMETERS(v);
			min=UNIT_ROUND_POINTS_TO_MILLIMETERS(ob->minpt);
			max=UNIT_ROUND_POINTS_TO_MILLIMETERS(ob->maxpt);
			step=MILLIMETERS_PRECISION;
			break;
		case UNIT_CENTIMETERS:
			v=UNIT_ROUND_POINTS_TO_CENTIMETERS(v);
			min=UNIT_ROUND_POINTS_TO_CENTIMETERS(ob->minpt);
			max=UNIT_ROUND_POINTS_TO_CENTIMETERS(ob->maxpt);
			step=CENTIMETERS_PRECISION;
			break;
		default:
			fprintf(stderr,"PANIC! Unit %d was not caught!\n",ob->unit);
			fprintf(stderr,"Unit definitions: %d, %d, %d, %d\n",UNIT_POINTS,UNIT_INCHES,UNIT_MILLIMETERS,UNIT_CENTIMETERS);
			break;
	}

//	fprintf(stderr,"Setting range to: %f, %f; value to %f; step to %f (unit %d)\n",min,max,v,step,ob->unit);
//	fprintf(stderr,"Sanity check:\n");
//	sanitycheck();
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->spinbutton),min,max);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->spinbutton),v);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(ob->spinbutton),step,step*10.0);
	dimension_update_unit_label(ob);
}


void dimension_set_pt(Dimension *ob,int pt)
{
	double v=0.0;
	if(pt==ob->value)
		return;

	switch(ob->unit)
	{
		case UNIT_POINTS:
			v=pt;
			break;
		case UNIT_INCHES:
			v=UNIT_ROUND_POINTS_TO_INCHES(pt);
			break;
		case UNIT_MILLIMETERS:
			v=UNIT_ROUND_POINTS_TO_MILLIMETERS(pt);
			break;
		case UNIT_CENTIMETERS:
			v=UNIT_ROUND_POINTS_TO_CENTIMETERS(pt);
			break;
	}

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->spinbutton),v);
	ob->value=v;
}


void dimension_set_value(Dimension *ob,gdouble val,enum Units unit)
{
	dimension_set_unit(ob,unit);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(ob->spinbutton),val);
}


void dimension_set_range_pt(Dimension *ob,int low,int high)
{
	ob->minpt=low;
	ob->maxpt=high;
	float ulow=low,uhigh=high;
	switch(ob->unit)
	{
		case UNIT_POINTS:
			break;
		case UNIT_INCHES:
			ulow=UNIT_ROUND_POINTS_TO_INCHES(low);
			uhigh=UNIT_ROUND_POINTS_TO_INCHES(high);
			break;
		case UNIT_MILLIMETERS:
			ulow=UNIT_ROUND_POINTS_TO_MILLIMETERS(low);
			uhigh=UNIT_ROUND_POINTS_TO_MILLIMETERS(high);
			break;
		case UNIT_CENTIMETERS:
			ulow=UNIT_ROUND_POINTS_TO_CENTIMETERS(low);
			uhigh=UNIT_ROUND_POINTS_TO_CENTIMETERS(high);
			break;
		default:
			fprintf(stderr,"PANIC! Unit %d was not caught!\n",ob->unit);
			fprintf(stderr,"Unit definitions: %d, %d, %d, %d\n",UNIT_POINTS,UNIT_INCHES,UNIT_MILLIMETERS,UNIT_CENTIMETERS);
			break;
	}
	fprintf(stderr,"Setting widget range to: %f -> %f\n",ulow,uhigh);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(ob->spinbutton),ulow,uhigh);
}


void dimension_show_unit(Dimension *ob)
{
	gtk_widget_show(ob->label);
}

void dimension_hide_unit(Dimension *ob)
{
	gtk_widget_hide(ob->label);
}
