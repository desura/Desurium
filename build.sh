#!/bin/bash
# Run from root repo dir
# Build desura
source env.sh
cd src/third_party
./SvnCheckOut.sh
./BuildAll.sh
cd ../../build_lin
./build.sh
