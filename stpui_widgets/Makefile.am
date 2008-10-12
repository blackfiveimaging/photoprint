AM_CFLAGS = -Wall $(GTK2_CFLAGS) $(GP_CFLAGS)
AM_CXXFLAGS = -Wall $(GTK2_CFLAGS) $(GP_CFLAGS)

noinst_LTLIBRARIES = libstpui_widgets.la

libstpui_widgets_la_SOURCES = 		\
	dimension.c				\
	dimension.h				\
	stpui_combo.c			\
	stpui_combo.h			\
	stpui_optionbook.c		\
	stpui_optionbook.h		\
	stpui_optionpage.c		\
	stpui_optionpage.h		\
	stpui_printerselector.c		\
	stpui_printerselector.h		\
	stpui_slider.c			\
	stpui_slider.h			\
	stpui_toggle.c			\
	stpui_toggle.h			\
	stpui_file.c			\
	stpui_file.h			\
	stpui_queue.c			\
	stpui_queue.h			\
	stputil.c				\
	stputil.h				\
	units.h

#	stpui_combo_gtk2.4.c		\
#	stpui_combo_gtk2.4.h		\
#	stpui_printerselector_gtk2.4.c	\
#	stpui_printerselector_gtk2.4.h


libstpui_widgets_la_LDFLAGS = -static

check_PROGRAMS = test activetest

test_SOURCES = test.cpp
test_DEPENDENCIES = libstpui_widgets.la ../stp_support/libstp_support.la
test_LDADD = -L. -lstpui_widgets -L../stp_support/ -lstp_support $(GTK2_LIBS) $(GP_LIBS)

activetest_SOURCES = activetest.cpp
activetest_LDADD = $(GP_LIBS)
