# Makefile.am
# RL 28/11/04

SUBDIRS = po support layoutsupport imagesource imageutils miscwidgets stpui_widgets stp_support gp_cppsupport pixbufthumbnail \
	splashscreen profilemanager effects borders shortcuts

localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I./layoutsupport -I./support/ -I./miscwidgets -I./imagesource/  \
	-I./splashscreen -I./profilemanager -I./effects -I./miscwidgets \
	$(LCMS_CFLAGS) $(GP_CFLAGS) $(GTK2_CFLAGS)

noinst_LTLIBRARIES = libphotoprint.la libphotoprintui.la

libphotoprint_la_SOURCES = \
	layoutdb.h		\
	layout.cpp		\
	layout.h		\
	layout_imageinfo.cpp \
	layout_imageinfo.h \
	layout_carousel.cpp	\
	layout_carousel.h	\
	layout_nup.cpp	\
	layout_nup.h	\
	layout_poster.cpp	\
	layout_poster.h	\
	layout_single.cpp	\
	layout_single.h	\
	histogram.cpp \
	histogram.h	\
	photoprint_state.cpp	\
	photoprint_state.h


libphotoprintui_la_SOURCES = \
	pp_cms.cpp		\
	pp_cms.h		\
	pp_imagecontrol.cpp	\
	pp_imagecontrol.h	\
	pp_mainwindow.cpp	\
	pp_mainwindow.h		\
	pp_menu_file.cpp	\
	pp_menu_file.h	\
	pp_menu_edit.cpp	\
	pp_menu_edit.h	\
	pp_menu_layout.cpp	\
	pp_menu_layout.h	\
	pp_menu_image.cpp	\
	pp_menu_image.h	\
	pp_menu_options.cpp	\
	pp_menu_options.h	\
	pp_menu_shortcuts.cpp	\
	pp_menu_shortcuts.h	\
	pp_menu_help.cpp	\
	pp_menu_help.h	\
	pp_layout_carousel.cpp	\
	pp_layout_carousel.h	\
	pp_layout_carousel_pageview.cpp		\
	pp_layout_carousel_pageview.h		\
	pp_layout_nup.cpp	\
	pp_layout_nup.h	\
	pp_layout_nup_pageview.cpp		\
	pp_layout_nup_pageview.h		\
	pp_layout_poster.cpp	\
	pp_layout_poster.h	\
	pp_layout_poster_pageview.cpp	\
	pp_layout_poster_pageview.h	\
	pp_layout_single.cpp	\
	pp_layout_single.h	\
	pp_layout_single_pageview.cpp	\
	pp_layout_single_pageview.h		\
	pp_histogram.cpp \
	pp_histogram.h \
	pp_pageextent.cpp	\
	pp_pageextent.h	\
	pp_scaling.cpp	\
	pp_scaling.h	\
	pp_sigcontrol.cpp	\
	pp_sigcontrol.h		\
	pp_units.cpp \
	pp_units.h \
	pp_imageinfo.cpp	\
	pp_imageinfo.h	\
	printpreview.cpp	\
	printpreview.h


bin_PROGRAMS = photoprint

photoprint_SOURCES = 		\
	dialogs.cpp		\
	dialogs.h		\
	photoprint.cpp		\
	cmtransformworker.h \
	gettext.h


LDADD = \
	libphotoprintui.la \
	libphotoprint.la \
	libphotoprintui.la \
	effects/libppeffects.la	\
	imagesource/libimagesource.la	\
	imageutils/libimageutils.la	\
	miscwidgets/libmiscwidgets.la	\
	profilemanager/libprofilemanager.la	\
	layoutsupport/liblayoutsupport.la	\
	support/libsupport.la	\
	gp_cppsupport/libgp_cppsupport.la \
	stpui_widgets/libstpui_widgets.la	\
	stp_support/libstp_support.la \
	pixbufthumbnail/libpixbufthumbnail.la	\
	splashscreen/libsplashscreen.la	\
	$(LIBINTL) $(LIBM_LIBS) $(GETOPT_LIBS) $(JPEG_LIBS) $(PNM_LIBS) $(TIFF_LIBS) $(LCMS_LIBS) $(GP_LIBS) $(GTK2_LIBS) $(X11_LIBS)

check_PROGRAMS = menucheck carouselcheck misccheck

menucheck_SOURCES = menucheck.cpp
carouselcheck_SOURCES = carouselcheck.cpp
misccheck_SOURCES = misccheck.cpp

imagesource/libimagesource.la:
	cd imagesource
	$(MAKE) libimagesource.la
	
support/libsupport.la:
	cd support
	$(MAKE) libsupport.la

profilemanager/libprofilemanager.la:
	cd profilemanager
	$(MAKE) libprofilemanager.la

stpui_widgets/libstpui_widgets.la:
	cd stpui_widgets
	$(MAKE) libstpui_widgets.la

stp_support/libstp_support.la:
	cd stp_support
	$(MAKE) libstp_support.la

gp_cppsupport/libgp_cppsupport.la:
	cd gp_cppsupport
	$(MAKE) libgp_cppsupport.la

pixbufthumbnail/libpixbufthumbnail.la:
	cd pixbufthumbnail
	$(MAKE) libpixbufthumbnail.la

effects/libppeffects.la:
	cd effects
	$(MAKE)	libppeffects.la

splashscreen/libsplashscreen.la:
	cd splashscreen
	$(MAKE) libsplashscreen.la


desktopdir = $(datadir)/applications
desktop_DATA = fotoprint.desktop

icondir = $(datadir)/icons/hicolor/48x48/apps/
icon_DATA = fotoprint.png


EXTRA_DIST = COPYING README INSTALL TODO photoprint.1 fotoprint.desktop fotoprint.png

