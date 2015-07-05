#!/bin/sh
libtoolize -f
gettextize -f
aclocal -I m4
automake --add-missing
autoheader
autoconf


