#/bin/sh
ln -fs /usr/bin/python2 ${1}/python
DEPOT_TOOLS_PATH=${1}
shift
PATH="${DEPOT_TOOLS_PATH}:$PATH" $@
