#!/bin/sh
DIR="./build/wxWidget-2-9-prefix/src/wxWidget-2-9/"
if [ -d "${DIR}" ] ; then
	cd ${DIR}
	if [[ ! -z `svn status | head -n1` ]] ; then
		echo "Reverting patches..."
		svn revert */*/*/* */*/*
	fi
	cd ../../../../build
fi
echo "Making clean..."
make clean
if [ -d install ] ; then
	echo "Removing install directory..."
	rm -rf install
fi
echo "Done"
