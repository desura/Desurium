#!/bin/bash

# Stolen from http://stackoverflow.com/a/246128/292831
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ] ; do SOURCE="$(readlink "$SOURCE")"; done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

if [[ "@RUNTIME_LIB_INSOURCE_DIR@" = /* ]]; then
	LD_LIBRARY_PATH="@RUNTIME_LIB_INSOURCE_DIR@/" @RUNTIME_LIB_INSOURCE_DIR@/desura $@
else
	LD_LIBRARY_PATH="$DIR/@RUNTIME_LIB_INSOURCE_DIR@/" $DIR/@RUNTIME_LIB_INSOURCE_DIR@/desura $@
fi

