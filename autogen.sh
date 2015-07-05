#!/bin/sh
libtoolize -f -i
gettextize -f --no-changelog
cp /usr/share/gettext/gettext.h gettext.h
aclocal -I m4
automake --add-missing
autoheader
autoconf


