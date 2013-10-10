#!/bin/bash
#
# Warning: This file is auto generated. Do not edit
# (Template: 0 is bin path, 1 is args, 2 is lib path)
#

cd "`dirname "$(readlink -f "$0")"`"

if [ -e "desura_prelaunch.sh" ]; then
	source desura_prelaunch.sh
fi

BIN_PATH="{0}"
ARGS={1}
LIB_PATH={2}

if [ -n $ARGS ]; then
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_PATH $BIN_PATH $ARGS $@
else
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_PATH $BIN_PATH $@
fi


