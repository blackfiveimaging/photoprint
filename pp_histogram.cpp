#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkimage.h>

#include "config.h"

#include "support/thread.h"
#include "stpui_widgets/dimension.h"
#include "imagesource/imagesource_util.h"

#include "photoprint_state.h"

#include "pp_histogram.h"

#include "gettext.h"
#define _(x) gettext(x)


static void pp_histogram_class_init (pp_HistogramClass *klass);
static void pp_histogram_init (pp_Histogram *stpuicombo);


void pp_histogram_refresh(pp_Histogram *ob)
{
	cerr << "Refreshing histogram" << endl;
	if(ob->layout)
	{
		Layout_ImageInfo *ii=ob->layout->FirstSelected();
		if(ii)
		{
			// Histogram
			PPHistogram &hist=ii->GetHistogram();
			if(hist.AttemptMutexShared())
			{
				try
				{
					int width=ob->hist->allocation.width;
					if(width>50)
					{
						GdkPixbuf *pb=hist.DrawHistogram(width,(2*width)/3);
						gtk_image_set_from_pixbuf(GTK_IMAGE(ob->hist),pb);
						g_object_unref(G_OBJECT(pb));
					}
				}
				catch(const char *err)
				{
					gtk_image_clear(GTK_IMAGE(ob->hist));
				}
				hist.ReleaseMutex();
			}
			else
			{
				gtk_image_clear(GTK_IMAGE(ob->hist));
				// Histogram is still being built...				
			}
		}
		else
		{
			gtk_image_clear(GTK_IMAGE(ob->hist));
		}
	}
}


static void expander_callback (GObject *object, GParamSpec *param_spec, gpointer userdata)
{
	pp_Histogram *ob=PP_HISTOGRAM(object);
	ob->layout->state.SetInt("ExpanderState_Histogram",gtk_expander_get_expanded (GTK_EXPANDER(ob)));
}


GtkWidget*
pp_histogram_new (Layout *layout)
{
	pp_Histogram *ob=PP_HISTOGRAM(g_object_new (pp_histogram_get_type (), NULL));
//	gtk_box_set_spacing(GTK_BOX(ob),5);
	ob->layout=layout;
	
	// Layout frame

	gtk_expander_set_label(GTK_EXPANDER(ob),_("Histogram"));
	gtk_expander_set_spacing(GTK_EXPANDER(ob),5);
	gtk_expander_set_expanded(GTK_EXPANDER(ob),layout->state.FindInt("ExpanderState_Histogram"));
	g_signal_connect(ob, "notify::expanded",G_CALLBACK (expander_callback), NULL);


	GtkWidget *vbox=gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox);


	ob->hist=gtk_image_new();
	gtk_container_add(GTK_CONTAINER(ob),ob->hist);
	gtk_widget_show(ob->hist);

	pp_histogram_refresh(ob);

	return(GTK_WIDGET(ob));
}


GType
pp_histogram_get_type (void)
{
	static GType stpuic_type = 0;

	if (!stpuic_type)
	{
		static const GTypeInfo pp_histogram_info =
		{
			sizeof (pp_HistogramClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) pp_histogram_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (pp_Histogram),
			0,
			(GInstanceInitFunc) pp_histogram_init,
		};
		stpuic_type = g_type_register_static (GTK_TYPE_EXPANDER, "pp_Histogram", &pp_histogram_info, (GTypeFlags)0);
	}
	return stpuic_type;
}


static void
pp_histogram_class_init (pp_HistogramClass *klass)
{
}


static void
pp_histogram_init (pp_Histogram *ob)
{
	ob->hist=NULL;
	ob->layout=NULL;
}

