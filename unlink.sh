make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

for lib in $(ls .bin/Debug/*.so.*)
do 
	echo rm -f /usr/lib64/$(basename ${lib})
done

for lib in $(ls .bin/Debug/*.dll)
do 
	echo rm -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/$(basename ${lib})
done

#for lib in .bin/Debug/*.dll
#do 
#	echo rm -f /usr/x86_64-w64-mingw32/sys-root/mingw/bin/$(basename ${lib})
#done


