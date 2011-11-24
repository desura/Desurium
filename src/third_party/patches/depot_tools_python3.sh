#!/bin/bash

if [ ! -d "$PWD/../depot_tools" ]; then
	echo "Run this from the patches directory!" 1>&2
	exit
fi

# /usr/bin/python is explicity used in depot_tools, and we're fixing that.
PYTHON="/usr/bin/python"
PYTHONVER="`$PYTHON2 --version 2>&1`"

# Check if $PYTHON is Python 2..
if [ "`echo \"$PYTHONVER\" | grep \"Python 2\"`" == "" ]; then # It's not!
	# Clever hack borrowed from the official Arch Linux Chromium PKGBUILD.
	# The idea is that shebangs use absolute paths, which need to be changed.
	# Relative calls like 'python blah.py' get hijacked by the 'python' in $PATH.
	
	REALPYTHON="$(readlink `which python`)"
	
	cd ../depot_tools
	
	find . -type f -exec sed -i -r \
		-e "s|/usr/bin/python$|$REALPYTHON|g" \
		-e "s|/usr/bin/python2\.4$|$REALPYTHON|g" \
		{} +
else
	echo "`which $PYTHON` is Python 2, no need to patch." 1>&2
fi
