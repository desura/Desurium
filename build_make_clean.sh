#!/bin/sh
echo "Making clean..."

if [ -d "build" ] ; then
	cd ./build/
	make clean
	cd ../
fi

if [ -d "build_cef" ] ; then
	cd ./build_cef/
	make clean
	cd ../
fi
if [ -d install ] ; then
	echo "Removing install directory..."
	rm -rf install
fi

echo "Done"
