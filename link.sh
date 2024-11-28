#!/bin/bash

build
if [ "$?" != "0" ]; then
	exit -1
fi

VERSION=$(meson introspect --projectinfo .build | jq -r '.version' | cut -d. -f1-2)

if [ -e .build/*.so.${VERSION} ]; then
	sudo ln -sf $(readlink -f .build/*.so.${VERSION}) /usr/lib64
	if [ "$?" != "0" ]; then
		exit -1
	fi
	sudo ln -sf $(readlink -f .bin/Debug/*.a) /usr/lib64
	if [ "$?" != "0" ]; then
		exit -1
	fi
fi

if [ -e .bin/Debug/*.dll ]; then
	sudo ln -sf $(readlink -f .bin/Debug/*.dll) /usr/x86_64-w64-mingw32/sys-root/mingw/bin
	if [ "$?" != "0" ]; then
		exit -1
	fi
fi
	
