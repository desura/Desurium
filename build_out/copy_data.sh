#!/bin/bash

#expects target dir as $1

export LC_ALL="POSIX"
export LD_LIBRARY_PATH="$PWD/../src/third_party/boost/stage/lib:/usr/local/lib64"

rm -fr $1/data

if [ -e ../tools/process_data_folder ]; then
	../tools/process_data_folder data $1/data _lin _win
	../tools/process_data_folder ../src/branding/data $1/data _lin _win
fi
