#!/bin/sh
export PATH="`pwd`/install:$PATH"
export LD_LIBRARY_PATH="`pwd`/install/lib:`pwd`/ceflibs"
desura
