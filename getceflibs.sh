#!/bin/sh
# -------
# Copyright (c) 2012 Jookia
#
# Usage of the works is permitted provided that this instrument is retained with
# the works, so that any entity that uses the works is notified of this
# instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

# we need mcf_extract, if we don't have it, compile...
if [ ! -e ./build/src/tools/mcf_extract/mcf_extract ] ; then
	echo "mcf_extract not found."
	echo "Compiling..."
	./build_cmake.sh
fi

COPYPATH="`pwd`/ceflibs/"

INSTALLDIR="$1"
if [ -z ${INSTALLDIR} ] ; then
	INSTALLDIR="install"
fi

# To get these, download the official Desura client, run it, and steal it from
# the XML file it downloads.
if [[ "`uname -m`" = "x86_64" ]]; then
	URL="http://app.desura.com/120/2011/1535.mcf"
elif [[ "`uname -m`" = "i686" ]]; then
	URL="http://app.desura.com/110/2011/1534.mcf"
else
	echo "Sorry, official Desura only supports x86_64 and i686. This won't work."
	exit 1
fi
if [ ! -d "${COPYPATH}" ] ; then
	mkdir $COPYPATH
fi
echo "Downloading libs..."
wget $URL -O desura.mcf
export LD_LIBRARY_PATH="`pwd`/${INSTALLDIR}/lib"
echo "Extracting libs..."
./build/src/tools/mcf_extract/mcf_extract desura.mcf tmp_desura

cd tmp_desura
mv lib_extra/* lib

# copyDeps recursively copies libraries. Could come in useful somewhere else.
# $1:        The library to copy, and to have its deps copied.
# $COPYPATH: The path to copy $1 to.
# $LIBSPATH: The path to search for deps of $1 in.
function copyDeps
{
	cp $1 $COPYPATH

	AWKLIBSPATH="`echo \"$LIBSPATH\" | sed 's|\/|\\\/|g'`"

	for dep in $(ldd 2>/dev/null $1 | awk "/^.*$AWKLIBSPATH.*$/{print \$1}")
	do
		DEPLIBSPATH="$LIBSPATH/$dep"
		DEPCOPYPATH="$COPYPATH/$dep"

		if [[ ! -f $DEPCOPYPATH ]]; then
			copyDeps $DEPLIBSPATH
		fi
	done
}

LIBSPATH="${INSTALLDIR}/lib"
#export LD_LIBRARY_PATH="/lib:/usr/lib:$LIBSPATH"
export LD_LIBRARY_PATH="/opt/desura/lib"
echo "Copying libs to destinations..."
copyDeps "$LIBSPATH/libcef_desura.so"

echo "Removing unwanted files..."
cd ..
rm -r ./tmp_desura/ desura.mcf
echo "Done"
