#!/bin/sh

aclocal
libtoolize
automake --gnu --add-missing --copy
autoconf
