#!/bin/sh
echo "reverting patches..."
cd ./build/wxWidget-2-9-prefix/src/wxWidget-2-9
svn revert */*/*/* */*/*
cd ../../../../build
echo "making clean"
make clean
echo "removing install directory"
rm -rf install
