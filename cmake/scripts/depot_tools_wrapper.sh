#/bin/sh
if [ -f "/usr/bin/python2" ]; then
	ln -fs /usr/bin/python2 ${1}/python
fi

DEPOT_TOOLS_PATH=${1}

# some mingw stuff
if [ ${DEPOT_TOOLS_PATH} = "C:/"* ]; then
	DEPOT_TOOLS_PATH="/c${DEPOT_TOOLS_PATH:2}"
fi

shift
PATH="${DEPOT_TOOLS_PATH}:$PATH" $@
