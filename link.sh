#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

if [ -e .bin/Debug/*.so.* ]; then
	sudo ln -sf $(readlink -f .bin/Debug/*.so.*) /usr/lib64
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
	
