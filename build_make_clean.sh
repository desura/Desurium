#!/bin/sh
echo "Making clean..."
cd ./build/
make clean
cd ../
if [ -d install ] ; then
	echo "Removing install directory..."
	rm -rf install
fi
echo "Done"
