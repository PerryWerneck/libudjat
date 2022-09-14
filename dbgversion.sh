#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

mkdir -p .bin/Debug/modules

ln -sf libudjat.so.1.0 .bin/Debug/libudjat.so

cat > .bin/libudjat.pc << EOF
prefix=/usr
exec_prefix=/usr
libdir=$(readlink -f .bin/Debug)
includedir=$(readlink -f ./src/include)
product_name=udjat
product_title=The Eye of Horus
module_path=$(readlink -f .bin/Debug)/modules
Name: udjat
Description: The Eye of Horus (DEBUG)
Version: 1.0
Libs: -L$(readlink -f .bin/Debug) -ludjat
Libs.private:  -ldl  -leconf -lpugixml -lsystemd -lvmdetect
Cflags: -I$(readlink -f ./src/include)
EOF

sudo ln -sf $(readlink -f .bin/libudjat.pc) /usr/lib64/pkgconfig/libudjat.pc

