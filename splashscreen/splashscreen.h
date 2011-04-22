#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <iostream>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

// Splash screen base class 
class SplashScreen
{
	public:
	SplashScreen()
	{
	}
	virtual ~SplashScreen()
	{
	}
	virtual void SetMessage(const char *msg)
	{
		std::cout << msg << std::endl;
	}
};


class SplashScreen_GTK : public SplashScreen
{
	public:
	SplashScreen_GTK();
	virtual ~SplashScreen_GTK();
	virtual void SetMessage(const char *msg);
	static GdkPixbuf *GetPixbuf();
	private:
	GtkWidget *window;
	GtkWidget *image;
	GtkWidget *message;
	GdkPixbuf *pixbuf;
};


#endif
