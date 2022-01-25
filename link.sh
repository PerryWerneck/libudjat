#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

sudo ln -sf $(readlink -f .bin/Debug/*.so.*) /usr/lib64
if [ "$?" != "0" ]; then
	exit -1
fi

#sudo rm -fr /usr/include/udjat
#sudo ln -sf $(readlink -f src/include/udjat) /usr/include/udjat
#if [ "$?" != "0" ]; then
#	exit -1
#fi

