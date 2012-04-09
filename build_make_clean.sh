#!/bin/sh
echo "reverting patches..."
cd ./build/wxWidget-2-9-prefix/src/wxWidget-2-9 || exit 1
svn revert */*/*/* */*/*
cd ../../../../build || exit 2
echo "making clean"
make clean
echo "removing install directory"
rm -rf install
