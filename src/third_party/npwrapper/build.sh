#!/bin/bash

target=Build32
libName=libdesura_flashwrapper_32.so
binName=desura_flashhost_32

if [ "x$1" == "x64" ]; then
	echo "Building 64 bit...."

	target=Build64
	libName=libdesura_flashwrapper_64.so
	binName=desura_flashhost_64
else
	echo "Building 32 bit...."
fi

rm -fr out/
rm *.mk
rm *Makefile

../gyp/gyp --depth=. flash_wrapper.gyp

make BUILDTYPE=$target

cp ./out/$target/flashhost ../../../build/lin_debug/bin/$binName
cp ./out/$target/flashhost ../../../build/lin_release/bin/$binName

cp ./out/$target/lib.target/libflashwrapper.so ../../../build/lin_debug/lib/$libName
cp ./out/$target/lib.target/libflashwrapper.so ../../../build/lin_release/lib/$libName
