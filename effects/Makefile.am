CLEANFILES = ppeffect_desaturate_icon.cpp ppeffect_temperature_icon.cpp

localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I../ -I../imagesource/ -I../pixbufthumbnail -I../support/ -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS)

noinst_LTLIBRARIES = libppeffects.la

libppeffects_la_SOURCES =	\
	effectlist.cpp	\
	effectlist.h	\
	effects_dialog.cpp	\
	effects_dialog.h	\
	effectselector.cpp	\
	effectselector.h	\
	effectwidget_tempchange.cpp	\
	effectwidget_tempchange.h	\
	effectwidget_unsharpmask.cpp	\
	effectwidget_unsharpmask.h	\
	effectwidget_gamma.cpp	\
	effectwidget_gamma.h	\
	effectwidget_channelmask.cpp	\
	effectwidget_channelmask.h	\
	ppeffect.cpp 	\
	ppeffect.h		\
	ppeffect_desaturate.cpp	\
	ppeffect_desaturate.h	\
	ppeffect_temperature.cpp	\
	ppeffect_temperature.h	\
	ppeffect_gamma.cpp	\
	ppeffect_gamma.h	\
	ppeffect_unsharpmask.cpp	\
	ppeffect_unsharpmask.h	\
	ppeffect_channelmask.cpp	\
	ppeffect_channelmask.h

libppeffects_la_LDFLAGS = -static

BUILT_SOURCES = ppeffect_temperature_icon.cpp ppeffect_desaturate_icon.cpp ppeffect_unsharpmask_icon.cpp ppeffect_gamma_icon.cpp

ppeffect_gamma_icon.cpp: GammaIcon.bmp
	gdk-pixbuf-csource --name=PPEffect_Gamma_Icon >ppeffect_gamma_icon.cpp GammaIcon.bmp

ppeffect_temperature_icon.cpp: TemperatureIcon.bmp
	gdk-pixbuf-csource --name=PPEffect_Temperature_Icon >ppeffect_temperature_icon.cpp TemperatureIcon.bmp

ppeffect_desaturate_icon.cpp: DesaturateIcon.bmp
	gdk-pixbuf-csource --name=PPEffect_Desaturate_Icon >ppeffect_desaturate_icon.cpp DesaturateIcon.bmp

ppeffect_unsharpmask_icon.cpp: UnsharpMaskIcon.bmp
	gdk-pixbuf-csource --name=PPEffect_UnsharpMask_Icon >ppeffect_unsharpmask_icon.cpp UnsharpMaskIcon.bmp

check_PROGRAMS = dialogcheck

dialogcheck_SOURCES = dialogcheck.cpp
dialogcheck_DEPENDENCIES = libppeffects.la ../pixbufthumbnail/libpixbufthumbnail.la \
	../imagesource/libimagesource.la ../profilemanager/libprofilemanager.la \
	../support/libsupport.la
dialogcheck_LDADD = -L. -lppeffects -L../imagesource -limagesource -L../profilemanager -lprofilemanager \
	-L../support -lsupport -L../pixbufthumbnail -lpixbufthumbnail  $(LIBINTL) $(TIFF_LIBS) $(LCMS_LIBS) $(GTK2_LIBS)

EXTRA_DIST = TemperatureIcon.bmp DesaturateIcon.bmp UnsharpMaskIcon.bmp \
		ppeffect_temperature_icon.cpp ppeffect_desaturate_icon.cpp ppeffect_unsharpmask_icon.cpp ppeffect_gamma_icon.cpp

