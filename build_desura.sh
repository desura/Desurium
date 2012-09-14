#!/bin/sh
printf  'Make sure to run \033[1;31msudo ./install-deps.sh\033[0m before compiling!\n'

printf 'We are compiling CEF first'
if [ ! -d "build_cef" ] ; then
 mkdir build_cef
fi
cd build_cef
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DRUNTIME_LIBDIR="lib" -DBUILD_ONLY_CEF=ON || exit 1
make install $@ || exit 2
cd -

printf 'Now we are compiling desurium'
if [ ! -d "build" ] ; then
 mkdir build
fi
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DBINDIR="" -DRUNTIME_LIBDIR="lib" -DDATADIR="" -DBUILD_CEF=OFF || exit 3
make install $@ || exit 4
printf 'Run \033[1;31m./install/desura\033[0m to start Desura!\n'

