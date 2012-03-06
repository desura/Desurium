#!/bin/sh
if [ ! -d "build" ] ; then
 mkdir build
fi
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make install $*
