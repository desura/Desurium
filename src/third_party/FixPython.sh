#!/bin/sh

PYTHON2="python"
PYTHONVER="`$PYTHON2 --version 2>&1`"

# Check if $PYTHON2 is Python 2..
if [ "`echo \"$PYTHONVER\" | grep \"Python 2\"`" == "" ]; then # It's not!
	PYTHON2="python2" # A good guess..
	
	if [ "`which $PYTHON2`" != "" ]; then # Will be "" if $PYTHON2 isn't found.
		cd depot_tools
		
		# Clever hacks borrowed from the official Arch Linux Chromium PKGBUILD.
		
		find . -type f -exec sed -i -r \
			-e 's|/usr/bin/python$|&2|g' \
			-e 's|(/usr/bin/python2)\.4$|\1|g' \
			{} +
		
		cd .. 
		
		mkdir "python2-path"
		ln -s `which $PYTHON2` "python2-path/python"
	fi
fi
