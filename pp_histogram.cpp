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


class DeferHistogram : public ThreadFunction
{
	public:
	DeferHistogram(pp_Histogram *widget,Layout_ImageInfo *ii) : ThreadFunction(), widget(widget),ii(ii), thread(this)
	{
		cerr << "Starting DeferHistogram thread" << endl;
		thread.Start();
		thread.WaitSync();
		cerr << "Startup confirmed" << endl;
	}
	virtual ~DeferHistogram()
	{
	}
	virtual int Entry(Thread &t)
	{
		// To avoid any possible deadlock situation we only attempt the mutex, and bail out if it fails.
		int count=10;
		while(!ii->AttemptMutexShared())
		{
			if(count==0)
			{
				cerr << "Giving up attempt on mutex - bailing out" << endl;
				// The calling thread is waiting for us to acknowledge startup, so we have to send
				// the Sync before bailing out.
				thread.SendSync();
				g_timeout_add(1,DeferHistogram::CleanupFunc,this);
				return(0);
			}
#ifdef WIN32
			Sleep(50);
#else
			usleep(50000);
#endif
			--count;
		}
		thread.SendSync();

		cerr << "DeferHistogram: Succeeded in obtaining ImageInfo mutex" << endl;
		// If we get this far we have a shared lock on the ImageInfo.

		PPHistogram &hist=ii->GetHistogram();
		cerr << "Subscribing to signal" << endl;
		hist.Subscribe();
		cerr << "Subscribed - attemping mutex" << endl;
		while(!hist.AttemptMutexShared())
		{
			cerr << "Couldn't get Histogram mutex - waiting..." << endl;
			hist.QueryAndWait();
			cerr << "Got signal from Histogram" << endl;
		}
		cerr << "Histogram mutex obtained" << endl;

		g_timeout_add(1,DeferHistogram::IdleFunc,this);
		cerr << "Handed control back to main thread..." << endl;
		thread.WaitSync();
		cerr << "Received signal to say main thread is done - cleaning up" << endl;
		hist.Unsubscribe();
		hist.ReleaseMutexShared();
		ii->ReleaseMutexShared();
		return(0);
	}
	// IdleFunc - runs on the main thread's context,
	// thus, can safely render into the UI.
	static gboolean IdleFunc(gpointer ud)
	{
		DeferHistogram *p=(DeferHistogram *)ud;
		PPHistogram &hist=p->ii->GetHistogram();
		cerr << "Drawing histogram" << endl;
		int width=p->widget->hist->allocation.width;
		if(width>50)
		{
			GdkPixbuf *pb=hist.DrawHistogram(width,(2*width)/3);
			gtk_image_set_from_pixbuf(GTK_IMAGE(p->widget->hist),pb);
			g_object_unref(G_OBJECT(pb));
			cerr << "Done" << endl;
		}
		cerr << "Signalling subthread" << endl;
		p->thread.SendSync();
		cerr << "Deleting payload" << endl;
		delete p;
		return(FALSE);
	}
	static gboolean CleanupFunc(gpointer ud)
	{
		DeferHistogram *p=(DeferHistogram *)ud;
		p->thread.SendSync();
		delete p;
		return(FALSE);
	}
	pp_Histogram *widget;
	Layout_ImageInfo *ii;
	Thread thread;
};


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
				hist.ReleaseMutexShared();
			}
			else
			{
				DeferHistogram *d=new DeferHistogram(ob,ii);
				hist.ReleaseMutexShared();
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

