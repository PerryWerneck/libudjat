#!/bin/bash

test -n "$srcdir" || srcdir=$(readlink -f $(dirname "$0"))
test -n "$srcdir" || srcdir=$(readlink -f .)

cd ${srcdir}

mkdir -p ./scripts

aclocal
if test $? != 0 ; then
	echo "aclocal failed."
	exit -1
fi

autoconf
if test $? != 0 ; then
	echo "autoconf failed."
	exit -1
fi

automake --add-missing 2> /dev/null | true





