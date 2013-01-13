#!/bin/sh

PREFIX="../install"
BINDIR=""
LIBDIR="lib"
DATADIR=""

printf 'Make sure to run \033[1;31msudo ./install-deps.sh\033[0m before compiling!\n\n'
printf 'We are compiling CEF first\n'
if [ ! -d "build_cef" ] ; then
 mkdir build_cef
fi
cd build_cef
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DRUNTIME_LIBDIR=$LIBDIR -DBUILD_ONLY_CEF=ON || exit 1
make install $@ || exit 2
cd -
printf "\n"
printf 'Now we are compiling desurium\n'
if [ ! -d "build" ] ; then
 mkdir build
fi
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DBINDIR=$BINDIR -DRUNTIME_LIBDIR=$LIBDIR -DDATADIR=$DATADIR -DBUILD_CEF=OFF || exit 3
make install $@ || exit 4
printf 'Run \033[1;31m./install/desura\033[0m to start Desura!\n'
