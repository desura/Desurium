#!/bin/sh

COPYPATH="`pwd`/ceflibs"

ARCH="`uname -m`"
wget http://www.desura.com/desura-${ARCH}.tar.gz
tar -xvf desura-${ARCH}.tar.gz
cd desura
./desura
mv lib_extra/* lib
LIBSPATH="`pwd`/lib"
mkdir $COPYPATH

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

export LD_LIBRARY_PATH="/lib:/usr/lib:$LIBSPATH"
copyDeps "$LIBSPATH/libcef_desura.so"

cd ..
rm desura -r
rm desura-${ARCH}.tar.gz
xdg-desktop-menu uninstall desura.desktop
xdg-desktop-menu uninstall desura-force.desktop
