AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall

noinst_LTLIBRARIES = libstp_support.la

libstp_support_la_SOURCES = 		\
	printerqueues.c				\
	printerqueues.h

#	stpui_combo_gtk2.4.c		\
#	stpui_combo_gtk2.4.h		\
#	stpui_printerselector_gtk2.4.c	\
#	stpui_printerselector_gtk2.4.h


libstp_support_la_LDFLAGS = -static

check_PROGRAMS = test

test_SOURCES = test.c
test_DEPENDENCIES = libstp_support.la
test_LDADD = -L. -lstp_support $(SPOOL_LIBS)

EXTRA_DIST = printerqueues_unix.c printerqueues_win32.c
