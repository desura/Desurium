#!/bin/bash

# Stolen from http://stackoverflow.com/a/246128/292831
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ] ; do SOURCE="$(readlink "$SOURCE")"; done
DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"

export PATH="$DIR:$PATH"
export LD_LIBRARY_PATH="$DIR/lib"
$@
