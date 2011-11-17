#!/bin/bash

# Modules used:
#   date_time
#   filesystem
#   thread
#   system

version="1.47.0"
target=boost
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

BUILDEXE=bjam

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

function bootstrapBoost()
{
	# TODO detect if bootstrap is needed!
	./bootstrap.sh 2>>$logFile 1>>$logFile
	bootstrapReturn=$?

	if [ -e b2 ]; then
		BUILDEXE=b2 # b2 is replacing bjam
	fi

	if [ $bootstrapReturn -ne 0 ]; then
		exitFailed "bootstrap"
	fi
}

function copyFiles()
{
	#$1 is dest
	mkdir -p ../../../build_out/$1/lib 2>>$logFile 1>>$logFile
	cp ./stage/lib/libboost_*-desura.so.$version ../../../build_out/$1/lib 2>>$logFile 1>>$logFile
}

function buildBoost()
{
	./$BUILDEXE stage --buildid=desura --with-date_time --with-filesystem --with-thread --with-system $2 2>>$logFile 1>>$logFile
	bjamReturn=$?
	
	if [ $bjamReturn -eq 0 ]; then
		copyFiles $1
	else
		exitFailed "$BUILDEXE $1"
	fi
}

function main()
{
	echo > $logFile
	outputStarting
	bootstrapBoost
	
	if [ x"$1" == x"--clean" ]; then
		echo "           ( cleaning )"
		./$BUILDEXE --clean debug release 2>>$logFile 1>>$logFile
		exitSuccess
	fi
	
	buildBoost release_lin
	#buildBoost lin_debug variant=debug

	# Hack to copy release boost to debug Desura
	copyFiles debug_lin
	
	exitSuccess
}
# ENTRY POINT
main $@
