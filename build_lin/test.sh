#!/bin/bash

origDir=$PWD
args="--debug $1"
destPath=debug_lin

if [ "$1" == '--release' -o "$1" == "-r" ]; then
	args=$2
	destPath=release_lin
fi

cd ../build_out/$destPath/
./desura $args

cd $origDir
