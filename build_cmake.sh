#!/bin/sh
mkdir src/build
cd src/build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make $*
