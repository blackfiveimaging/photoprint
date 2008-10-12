#include <iostream>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "pixbufview.h"

#include "config.h"
#include "gettext.h"
#define _(x) gettext(x)


using namespace std;

int main(int argc,char**argv)
{
	gtk_init(&argc,&argv);

	try
	{
		GtkWidget *win=gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (win), _("ImageTarget"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		GtkWidget *pview=pixbufview_new(NULL,false);
		gtk_container_add (GTK_CONTAINER (win), pview);
		gtk_widget_show(pview);
		gtk_widget_show(win);

		if(argc>1)
		{
			ImageSource *is=ISLoadImage(argv[1]);
			GdkPixbuf *pb=pixbuf_from_imagesource(is);
			pixbufview_set_pixbuf(PIXBUFVIEW(pview),pb);
//			g_object_unref(G_OBJECT(pview));
		}

		gtk_main();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

