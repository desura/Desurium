#!/bin/sh
echo "Making clean..."
make clean
if [ -d install ] ; then
	echo "Removing install directory..."
	rm -rf install
fi
echo "Done"
