#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "splashscreen.h"
#include "splashscreendata.cpp"


GdkPixbuf *SplashScreen::GetPixbuf()
{
	GdkPixdata pd;
	GdkPixbuf *result;
	GError *err;

	if(!gdk_pixdata_deserialize(&pd,sizeof(my_pixbuf),my_pixbuf,&err))
		throw(err->message);

	if(!(result=gdk_pixbuf_from_pixdata(&pd,false,&err)))
		throw(err->message);

	return(result);
}


SplashScreen::SplashScreen()
	: window(NULL), image(NULL), message(NULL), pixbuf(NULL)
{
	pixbuf=GetPixbuf();

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER_ALWAYS);
	gtk_window_set_decorated(GTK_WINDOW(window),FALSE);

	GtkWidget *box=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),box);

	image=gtk_image_new_from_pixbuf(pixbuf);
	gtk_box_pack_start(GTK_BOX(box),image,FALSE,FALSE,0);

	message=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(box),message,TRUE,FALSE,0);

	gtk_widget_show(message);
	gtk_widget_show(image);
	gtk_widget_show(box);
	gtk_widget_show(window);	
}


SplashScreen::~SplashScreen()
{
	if(window)
		gtk_widget_destroy(window);
	if(pixbuf)
		g_object_unref(G_OBJECT(pixbuf));
}


void SplashScreen::SetMessage(const char *msg)
{
	if(message)
		gtk_label_set_text(GTK_LABEL(message),msg);
	gtk_label_set_selectable(GTK_LABEL(message),TRUE);

	while(gtk_events_pending())
		gtk_main_iteration_do(false);
}
