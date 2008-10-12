#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

class SplashScreen
{
	public:
	SplashScreen();
	~SplashScreen();
	void SetMessage(const char *msg);
	static GdkPixbuf *GetPixbuf();
	private:
	GtkWidget *window;
	GtkWidget *image;
	GtkWidget *message;
	GdkPixbuf *pixbuf;
};

#endif
