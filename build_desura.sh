#!/bin/sh
echo -e "Make sure to run \e[1;31msudo install-deps.sh\e[0m before compiling!"
if [ ! -d "build" ] ; then
 mkdir build
fi
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install || exit 1
make install $@ || exit 2
echo -e "Run \e[1;31m./install/run.sh\e[0m to start Desura!"
