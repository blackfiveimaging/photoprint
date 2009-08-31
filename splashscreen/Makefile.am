AM_CXXFLAGS = -Wall $(GTK2_CFLAGS)

noinst_LTLIBRARIES = libsplashscreen.la

libsplashscreen_la_SOURCES =	\
	splashscreen.cpp	\
	splashscreen.h	\
	SplashScreen.tif

BUILT_SOURCES = splashscreendata.cpp

libsplashscreen_la_LDFLAGS = -static

splashscreendata.cpp:
	gdk-pixbuf-csource >splashscreendata.cpp SplashScreen.tif

EXTRA_DIST = splashscreendata.cpp
