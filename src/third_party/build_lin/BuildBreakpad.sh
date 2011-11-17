#!/bin/bash

target=breakpad
logFile=$PWD/$target.log
origDir=$PWD
cd ../$target

ce='\e[00m'		#color end
cgr='\e[01;30m'	#color bold grey
cr='\e[01;31m'  #color bold red
cg='\e[01;32m'	#color bold green
cy='\e[01;33m'	#color bold yellow
cb='\e[01;34m'	#color bold blue
cv='\e[01;35m'	#color bold violet
ca='\e[01;36m'	#color bold aqua
cw='\e[01;37m'	#color bold white

# If it's not globally defined (length 0)
if [ ${#MAKE_CONCURRENCY} -eq 0 ]; then
	MAKE_CONCURRENCY=1
fi

function outputStarting()
{
	echo -e " ----------------------------------------- "
	echo -e " ${cw}Building:${ce} $target (please wait) "
}

function exitFailed()
{
	# $1 == Component that failed
	echo -e " ${cw}Failed At:${ce} $1"
	echo -e " ${cw}Logfile:${ce} $logFile"
	echo -e " ${cw}Exit Status:${ce} 1 (${cr}FAILED${ce})"
	
	cd $origDir
	exit 1
}

function exitSuccess()
{
	echo -e " ${cw}Logfile:${ce} $logFile"
	echo -e " ${cw}Exit Status:${ce} 0 (${cg}SUCCESS${ce})"
	
	cd $origDir
	exit 0
}

function configureBreakpad()
{
	./configure --enable-shared=yes --enable-static=yes 2>>$logFile 1>>$logFile
	if [ $? -ne 0 ]; then
		exitFailed "configure"
	fi
}

function makeBreakpad()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	if [ $? -ne 0 ]; then
		exitFailed "make"
	fi
}

function main()
{
	echo > $logFile
	outputStarting

	if [ x"$1" == x"--clean" ]; then
		echo "           ( cleaning )"
		make clean 2>>$logFile 1>>$logFile
		exitSuccess
	fi

	configureBreakpad
	makeBreakpad

	exitSuccess
}

# ENTRY POINT
main $@
