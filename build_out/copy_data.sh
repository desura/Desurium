#!/bin/bash

#expects target dir as $1

export LC_ALL="POSIX"
export LD_LIBRARY_PATH="$PWD/../src/third_party/boost/stage/lib:/usr/local/lib64"

rm -fr $1/data

cd ..
echo "Working Dir: $PWD"

$PWD/tools/process_data_folder build_out/data build_out/$1/data _lin _win
$PWD/tools/process_data_folder src/branding/data build_out/$1/data _lin _win

