AM_CXXFLAGS = -Wall -I../imagesource/ -I../ -I../pixbufthumbnail -I../imagesource -I../profilemanager $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libsupport.la

libsupport_la_SOURCES =	\
	maskpixbuf.cpp	\
	maskpixbuf.h	\
	rotatepixbuf.cpp	\
	rotatepixbuf.h	\
	\
	consumer.cpp	\
	consumer.h	\
	configdb.cpp	\
	configdb.h	\
	\
	generaldialogs.cpp	\
	generaldialogs.h	\
	\
	layoutrectangle.cpp	\
	layoutrectangle.h	\
	pageextent.h	\
	signature.cpp	\
	signature.h	\
	circlemontage.cpp \
	circlemontage.h \
	\
	pathsupport.cpp \
	pathsupport.h \
	pixbufview.cpp \
	pixbufview.h \
	progress.h	\
	progresstext.h \
	progressthread.h \
	progressbar.cpp	\
	progressbar.h	\
	\
	rangeparser.cpp \
	rangeparser.h \
	\
	thread.cpp \
	thread.h \
	ptmutex.cpp \
	ptmutex.h \
	rwmutex.cpp \
	rwmutex.h \
	\
	searchpath.cpp \
	searchpath.h \
	searchpathdbhandler.h \
	patheditor.cpp	\
	patheditor.h	\
	imageselector.cpp \
	imageselector.h \
	imageselector_noborder_image.h \
	\
	jpegsave.cpp	\
	jpegsave.h	\
	tiffsave.cpp	\
	tiffsave.h

libsupport_la_LDFLAGS = -static

check_PROGRAMS = pathcheck jpegcheck pvtest threadtest

pathcheck_SOURCES = pathcheck.cpp
pathcheck_DEPENDENCIES = libsupport.la
pathcheck_LDADD = -L. -lsupport ${GTK2_LIBS}

jpegcheck_SOURCES = jpegcheck.cpp
jpegcheck_DEPENDENCIES = libsupport.la ../imagesource/libimagesource.la ../profilemanager/libprofilemanager.la 
jpegcheck_LDADD = -L. -lsupport -L../imagesource/ -limagesource -L../profilemanager -lprofilemanager ${LCMS_LIBS} ${JPEG_LIBS} ${TIFF_LIBS} ${GTK2_LIBS} ${PNM_LIBS}

pvtest_SOURCES = pvtest.cpp
pvtest_LDADD = ../imagesource/libimagesource.la libsupport.la ../profilemanager/libprofilemanager.la $(LIBINTL) $(LIBM_LIBS) $(JPEG_LIBS) $(PNM_LIBS) $(TIFF_LIBS) $(LCMS_LIBS) $(GTK2_LIBS)

threadtest_SOURCES = threadtest.cpp
threadtest_DEPENDENCIES = libsupport.la
threadtest_LDADD = -L. -lsupport ${GTK2_LIBS} ${GTHREAD_CFLAGS}


