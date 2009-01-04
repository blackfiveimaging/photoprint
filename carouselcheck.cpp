#include <iostream>

#include <gtk/gtk.h>

#include "imagesource/imagesource_util.h"
#include "imagesource/pixbuf_from_imagesource.h"
#include "imagesource/imagesource_montage.h"
#include "imagesource/imagesource_segmentmask.h"
#include "imagesource/imagesource_mask.h"
#include "miscwidgets/pixbufview.h"

#include "layout_carousel.h"

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

		GtkWidget *pview=pixbufview_new(NULL,false);
		gtk_container_add (GTK_CONTAINER (win), pview);
		gtk_widget_show(pview);
		gtk_widget_show(win);

		if(argc>1)
		{
			ImageSource_Montage *mon=new ImageSource_Montage(IS_TYPE_RGBA,300);
			CircleMontage c(512,512);
			c.SetSegments(6,25,25);
			c.SetInnerRadius(20);
			CMSegment *targetseg=c.GetSegmentExtent(0);
			ImageSource *is=ISLoadImage(argv[1]);
			ImageSource *mask=new ImageSource_SegmentMask(targetseg,true);
			is=ISScaleImageBySize(is,mask->width,mask->height);
			is=new ImageSource_Mask(is,mask);
			mon->Add(is,0,0);
			GdkPixbuf *pb=pixbuf_from_imagesource(mon,128,128,128);
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

