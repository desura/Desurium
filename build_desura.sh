#!/bin/sh
printf  'Make sure to run \033[1;31msudo install-deps.sh\033[0m before compiling!\n'
if [ ! -d "build" ] ; then
 mkdir build
fi
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install || exit 1
make install $@ || exit 2
printf 'Run \033[1;31m./install/run.sh\033[0m to start Desura!\n'
