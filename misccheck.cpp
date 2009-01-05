#include <iostream>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_montage.h"
#include "imagesource/imagesource_segmentmask.h"
#include "imagesource/imagesource_mask.h"
#include "miscwidgets/pixbufview.h"

#include "support/signature.h"
#include "stpui_widgets/units.h"
#include "pp_sigcontrol.h"

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
		gtk_window_set_title (GTK_WINDOW (win), _("PixBufView Test"));
		gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			(GtkSignalFunc) gtk_main_quit, NULL);

		Signature sig;
		sig.SetPaperSize(595,842);

//		GtkWidget *sc=pp_sigcontrol_new(&sig,UNIT_MILLIMETERS);
//		gtk_container_add(GTK_CONTAINER(win),sc);

		gtk_widget_show_all(win);

		gtk_main();
	}
	catch(const char *err)
	{
		cerr << "Error: " << err << endl;
	}

	return(0);
}

