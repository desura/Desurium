#!/bin/bash

target=chromium/src/cef
logFile=$PWD/`basename $target`.log
origDir=$PWD # should be third_party
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

function configureCEF()
{
	if [ "$2" != "--nogen" ]; then
		chmod a+x cef_create_projects.sh
		./cef_create_projects.sh 2>>$logFile 1>>$logFile
		ccpReturn=$?
		
		if [ $ccpReturn -ne 0 ]; then
			exitFailed "cef_create_projects"
		fi
	fi
}

function makeCEF()
{
	make cef_desura BUILDTYPE=$type -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	makeReturn=$?
	
	if [ $makeReturn -ne 0 ]; then
		exitFailed "make $type"
	fi
}

function main()
{
	echo > $logFile
	outputStarting

	if [ "$1" == "--debug" ]; then 
		type="Debug"
		out="debug_lin"
	else
		type="Release"
		out="release_lin"
	fi

	if [ x"$1" == x"--clean" ]; then
		echo "           ( cleaning )"
		rm -rf ../out 2>>$logFile 1>>$logFile
		exitSuccess
	fi
	
	configureCEF

	cd ..

	makeCEF

	mkdir -p ../../../../build_out/$out/lib/ 2>>$logFile 1>>$logFile
	mv out/$type/lib.target/libcef_desura.so ../../../../build_out/$out/lib/ 2>>$logFile 1>>$logFile
	mv out/$type/lib.target/libffmpegsumo.so ../../../../build_out/$out/lib/ 2>>$logFile 1>>$logFile

	exitSuccess
}

# ENTRY POINT
main $@
