#!/bin/bash
#
# References:
#
#	* https://www.msys2.org/docs/ci/
#
#
echo "Running ${0}"

LOGFILE=build.log
rm -f ${LOGFILE}

die ( ) {
	[ -s $LOGFILE ] && tail $LOGFILE
	[ "$1" ] && echo "$*"
	exit -1
}

cd $(dirname $(dirname $(readlink -f ${0})))

#
# Install pre-reqs
#
pacman -U --noconfirm *.pkg.tar.zst || die "pacman failure"

#
# Build
#
dos2unix PKGBUILD.mingw  || die "dos2unix failure"
makepkg BUILDDIR=/tmp/pkg -p PKGBUILD.mingw || die "makepkg failure"

echo "Build complete"

