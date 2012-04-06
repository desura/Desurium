#!/bin/bash
# -------
# Copyright (c) 2012 Jookia
#
# Usage of the works is permitted provided that this instrument is retained with
# the works, so that any entity that uses the works is notified of this
# instrument.
#
# DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

COPYPATH="`pwd`/ceflibs/"

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

mkdir $COPYPATH
wget $URL -O desura.mcf
export LD_LIBRARY_PATH="`pwd`/install/lib"
install/bin/mcf_extract desura.mcf tmp_desura

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

LIBSPATH="`pwd`/lib"
export LD_LIBRARY_PATH="/lib:/usr/lib:$LIBSPATH"
copyDeps "$LIBSPATH/libcef_desura.so"

cd ..
rm tmp_desura -r
rm desura.mcf
