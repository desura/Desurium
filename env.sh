#!/bin/bash

# It's okay to put src/third_party/ stuff in the PATH if they don't exist.
export PATH="$PATH:$PWD/src/third_party/depot_tools"
export PYTHONPATH="$PYTHONPATH:$PWD/src/third_party/gyp/pylib/"

# ---------- PYTHON3 SUPPORT!
# You still need to apply the patch in src/third_party/patches, though.

PYTHON="python"
PYTHONVER="`$PYTHON --version 2>&1`"

# Check if $PYTHON is Python 2..
if [ "`echo \"$PYTHONVER\" | grep \"Python 2\"`" == "" ]; then # It's not!
	PYTHON2="python2" # A good guess at the executable name.
	
	# Check if $PYTHON2 is found (only check which's stdout, stderr will give
	# errors), if it is then create a python2 -> python symlink and hijack $PATH.
	if [ "`which $PYTHON2 2>/dev/null`" != "" ]; then
		# Instead of forcing ln to do something, I'm silencing the error. This way
		# the risk of breaking things is minimal.
		ln -s `which $PYTHON2` "python" 2>/dev/null
		export PATH="$PWD:$PATH"
	else
		echo "`which python` isn't Python 2, and $PYTHON2 couldn't be"\
			"found. It's up to you now." 1>&2
	fi
fi

