#!/bin/bash

# Stolen from http://stackoverflow.com/a/246128/292831
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ] ; do SOURCE="$(readlink "$SOURCE")"; done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

# export PATH with DIR so we add the binary folder to PATH (for desura_bittest calls)
export PATH="$DIR:$PATH"

if [[ "@RUNTIME_LIB_INSOURCE_DIR@" = /* ]]; then
	LD_LIBRARY_PATH="@RUNTIME_LIB_INSOURCE_DIR@/" @RUNTIME_LIB_INSOURCE_DIR@/desura $@
else
	LD_LIBRARY_PATH="$DIR/@RUNTIME_LIB_INSOURCE_DIR@/" $DIR/@RUNTIME_LIB_INSOURCE_DIR@/desura $@
fi

