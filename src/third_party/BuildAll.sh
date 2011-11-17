#!/bin/bash

ce='\e[00m'		#color end
cgr='\e[01;30m'	#color bold grey
cr='\e[01;31m'  #color bold red
cg='\e[01;32m'	#color bold green
cy='\e[01;33m'	#color bold yellow
cb='\e[01;34m'	#color bold blue
cv='\e[01;35m'	#color bold violet
ca='\e[01;36m'	#color bold aqua
cw='\e[01;37m'	#color bold white

if [ "$1" == "--exit" ]; then
	exitFirstFail="true"
	shift
else
	exitFirstFail="false"
fi 

if [ x"$1" == x"--clean" ]; then
	echo -e " ----------------------------------------- "
	echo -e "  (cleaning)"
	EXTRAARGS="--clean"
fi

if [ "$1" == "--cleanlogs" ]; then
	rm -rf *.log
	exit
fi

cd build_lin

./BuildBoost.sh $EXTRAARGS
boostReturn=$?
if [ $boostReturn -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

./BuildBreakpad.sh $EXTRAARGS
breakpadReturn=$?
if [ $breakpadReturn -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

./BuildCurl.sh $EXTRAARGS
curlReturn=$?
if [ $curlReturn -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

./BuildV8.sh $EXTRAARGS
v8Return=$?
if [ $v8Return -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

./BuildWxWidgets.sh $EXTRAARGS
wxReturn=$?
if [ $wxReturn -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

./BuildCef.sh $EXTRAARGS
cefReturn=$?
if [ $cefReturn -ne 0 ]; then
	if [ "$exitFirstFail" == "true" ]; then
		exit 1
	fi
fi

cd ..

function colourOutput()
{
	if [ $1 -eq 0 ]; then
		echo -e "${cg}Success${ce}"
	else
		echo -e "${cr}Failed${ce}"
	fi
}

echo -e " ----------------------------------------- "
echo -e " ${cw}Summary:${ce}"
echo -ne "\tBoost:\t\t"; colourOutput $boostReturn
echo -ne "\tBreakpad:\t"; colourOutput $breakpadReturn
echo -ne "\tCurl:\t\t"; colourOutput $curlReturn
echo -ne "\tV8:\t\t"; colourOutput $v8Return
echo -ne "\twx:\t\t"; colourOutput $wxReturn
echo -ne "\tCef:\t\t"; colourOutput $cefReturn
echo -e " ----------------------------------------- "


