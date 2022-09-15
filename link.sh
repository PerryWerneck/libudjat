#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

sudo ln -sf $(readlink -f .bin/Debug/*.so.*) /usr/lib64
if [ "$?" != "0" ]; then
	exit -1
fi

