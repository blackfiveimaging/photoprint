#include <string.h>

#include <gtk/gtkframe.h>
#include <gtk/gtkexpander.h>
#include <gtk/gtksizegroup.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkimage.h>

#include "config.h"

#include "support/thread.h"
#include "support/debug.h"

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
		Debug[TRACE] << "Main thread: Starting DeferHistogram thread" << endl;
		thread.Start();
		Debug[TRACE] << "Main thread: Thread started - waiting for Sync" << endl;
		// FIXME - need to use bi-directional sync signals - can't use the same signal in each direction
		// because there's no guarantee the main thread will receive the signal before the subthread waits
		// for the next one.
		thread.WaitSync();
		Debug[TRACE] << "Main thread: Startup confirmed" << endl;
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
				Debug[TRACE] << "DeferHistogram: Giving up attempt on mutex - bailing out" << endl;
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

		Debug[TRACE] << "DeferHistogram: Succeeded in obtaining ImageInfo mutex" << endl;
		// If we get this far we have a shared lock on the ImageInfo.

		PPHistogram &hist=ii->GetHistogram();
		Debug[TRACE] << "DeferHistogram: Subscribing to signal" << endl;
		hist.Subscribe();
		Debug[TRACE] << "DeferHistogram: Subscribed - attemping mutex" << endl;
		if(!hist.AttemptMutexShared())
		{
			Debug[TRACE] << "DeferHistogram: Couldn't get Histogram mutex - waiting..." << endl;
			hist.QueryAndWait();
			Debug[TRACE] << "DeferHistogram: Got signal from Histogram - obtaining Mutex..." << endl;
			hist.ObtainMutexShared();
		}
		Debug[TRACE] << "DeferHistogram: Histogram mutex obtained" << endl;

		g_timeout_add(1,DeferHistogram::IdleFunc,this);
		Debug[TRACE] << "DeferHistogram: Handed control back to main thread..." << endl;
		thread.WaitSync();
		Debug[TRACE] << "DeferHistogram: Received signal to say main thread is done - cleaning up" << endl;
		hist.Unsubscribe();
		Debug[TRACE] << "DeferHistogram: Unsubscribed" << endl;
		hist.ReleaseMutexShared();
		ii->ReleaseMutexShared();
		Debug[TRACE] << "DeferHistogram: Mutexes released - subthread ending" << endl;
		return(0);
	}
	// IdleFunc - runs on the main thread's context,
	// thus, can safely render into the UI.
	static gboolean IdleFunc(gpointer ud)
	{
		DeferHistogram *p=(DeferHistogram *)ud;
		PPHistogram &hist=p->ii->GetHistogram();
		Debug[TRACE] << "DeferHistogram - IdleFunc: Drawing histogram" << endl;
		int width=p->widget->hist->allocation.width;
		if(width>50)
		{
			GdkPixbuf *pb=hist.DrawHistogram(width,(2*width)/3);
			gtk_image_set_from_pixbuf(GTK_IMAGE(p->widget->hist),pb);
			g_object_unref(G_OBJECT(pb));
			Debug[TRACE] << "Done" << endl;
		}
		Debug[TRACE] << "DeferHistogram - IdleFunc: Signalling subthread" << endl;
		p->thread.SendSync();
		Debug[TRACE] << "DeferHistogram - IdleFunc: Deleting payload" << endl;
		delete p;
		return(FALSE);
	}
	static gboolean CleanupFunc(gpointer ud)
	{
		DeferHistogram *p=(DeferHistogram *)ud;
//		p->thread.SendSync();
		delete p;
		return(FALSE);
	}
	pp_Histogram *widget;
	Layout_ImageInfo *ii;
	Thread thread;
};


class BuildHistogramThread : public ThreadFunction
{
	public:
	BuildHistogramThread(pp_Histogram *widget,Layout_ImageInfo *ii) : ThreadFunction(), widget(widget), ii(ii), thread(this)
	{
		Debug[TRACE] << "BuildHistogramThread: Starting Histogram generation thread" << endl;
		thread.Start();
		thread.WaitSync();
		Debug[TRACE] << "BuildHistogramThread: Startup confirmed" << endl;
	}
	virtual ~BuildHistogramThread()
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
				Debug[WARN] << "BuildHistogramThread: Giving up attempt on mutex - bailing out" << endl;
				// The calling thread is waiting for us to acknowledge startup, so we have to send
				// the Sync before bailing out.
				thread.SendSync();
				g_timeout_add(1,BuildHistogramThread::CleanupFunc,this);
				return(0);
			}
#ifdef WIN32
			Sleep(50);
#else
			usleep(50000);
#endif
			--count;
		}
		ISDataType *junk;
		ImageSource *is=ii->GetImageSource();

		Debug[TRACE] << "BuildHistogramThread: Succeeded in obtaining ImageInfo mutex" << endl;

		// If we get this far we have a shared lock on the ImageInfo.
		// We have also created the ImageSource chain, so should have a write lock on the histogram.

		thread.SendSync();

		// We only bother to read the image data if we have an exclusive lock on the histogram.
		if(ii->GetHistogram().CheckExclusive())
		{
			new DeferHistogram(widget,ii);

			for(int y=0;y<is->height;++y)
			{
				junk=is->GetRow(y);
			}
		}

		delete is;

		g_timeout_add(1,BuildHistogramThread::CleanupFunc,this);
		Debug[TRACE] << "BuildHistogramThread: Handed control back to main thread..." << endl;
		thread.WaitSync();
		Debug[TRACE] << "BuildHistogramThread: Received signal to say main thread is done - cleaning up" << endl;
		ii->ReleaseMutexShared();
		return(0);
	}
	static gboolean CleanupFunc(gpointer ud)
	{
		BuildHistogramThread *p=(BuildHistogramThread *)ud;
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
	Debug[TRACE] << "Main thread: Refreshing histogram" << endl;
	if(ob && ob->layout)
	{
		LayoutIterator it(*ob->layout);
		Layout_ImageInfo *ii=it.FirstSelected();
		if(ii)
		{
			// Histogram
			PPHistogram &hist=ii->GetHistogram();

			Debug[TRACE] << "Main thread: Got histogram" << endl;

			if(hist.AttemptMutexShared())
			{
				Debug[TRACE] << "Main thread: Got histogram's mutex - attempting to draw" << endl;
				// If we were able to obtain the histogram's mutex, then it's either
				// not been generated yet, or it's generated and ready for display...
				try
				{
					int width=ob->hist->allocation.width;
					if(width>50)
					{
						GdkPixbuf *pb=hist.DrawHistogram(width,(2*width)/3);
						gtk_image_set_from_pixbuf(GTK_IMAGE(ob->hist),pb);
						g_object_unref(G_OBJECT(pb));
					}
					hist.ReleaseMutexShared();
				}
				catch(const char *err)
				{
					// If drawing the histogram failed, we'll process it ourselves...
					Debug[TRACE] << "Main thread: Couldn't draw histogram - spawning thread to build it..." << endl;
					hist.ReleaseMutexShared();
					new BuildHistogramThread(ob,ii);
				}
			}
			else
			{
				// If we couldn't obtain the Histogram's mutex, then histogram generation must
				// be in progress - so launch a thread to wait for it...
				Debug[TRACE] << "Main thread: couldn't obtain histogram's mutex - Deferring..." << endl;
				new DeferHistogram(ob,ii);
				Debug[TRACE] << "Main thread: DeferHistogram created..." << endl;
			}
		}
		else
		{
			gtk_image_clear(GTK_IMAGE(ob->hist));
		}
	}
	Debug[TRACE] << "Main thread: Refresh complete." << endl;
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

