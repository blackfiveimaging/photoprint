#include <stdio.h>

#include "printerqueues.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include "printerqueues_win32.c"
#else
#include "printerqueues_unix.c"
#endif

