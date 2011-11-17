#!/bin/bash

target=wxWidgets
logFile=$PWD/$target.log
origDir=$PWD
#cd $target		# not needed for wxWidgets script

cd ..

ce='\e[00m'		#color end
cgr='\e[01;30m'	#color bold grey
cr='\e[01;31m'  #color bold red
cg='\e[01;32m'	#color bold green
cy='\e[01;33m'	#color bold yellow
cb='\e[01;34m'	#color bold blue
cv='\e[01;35m'	#color bold violet
ca='\e[01;36m'	#color bold aqua
cw='\e[01;37m'	#color bold white

#need to disable threads as it causes hangs in copy/paste with cef
#joystick needs threads so disable that to
CONFIG_OPTIONS="--enable-shared --enable-unicode --enable-monolithic --with-flavour=desura --disable-threads --disable-joystick"

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

function patchWx()
{
	cd wxWidgets
	
	which patch > /dev/null
	if [ $? -ne 0 ]; then
		echo "Patch not installed. Calling installer and restarting" 2>>$logFile 1>>$logFile
		sudo apt-get install patch 2>>$logFile 1>>$logFile
		if [ $? -ne 0 ]; then
			exitFailed "patch install"
		fi
		exec $0
	fi

	if [ ! -e patched ]; then
		patch -p0 -i ../patches/wxWidgets_patch.patch 2>>$logFile 1>>$logFile
		
		if [ $? -ne 0 ]; then
			exitFailed "patch apply"
		fi
		
		touch patched
	fi
	
	cd ..
}

function copyToDebug()
{
	if [ ! -d wxWidgets_debug ]; then
		cp -r wxWidgets wxWidgets_debug 2>>$logFile 1>>$logFile
		
		if [ $? -ne 0 ]; then
			exitFailed "copy to debug"
		fi
	fi
}



function configureRelease()
{
	cd wxWidgets

	# Builds shared and unicode by default, and vendor info isn't valid in Linux
	./configure $CONFIG_OPTIONS 2>>$logFile 1>>$logFile
	 
	if [ $? -ne 0 ]; then
		exitFailed "configure release"
	fi
}

function makeRelease()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile

	if [ $? -ne 0 ]; then
		exitFailed "make release"
	fi

	mkdir -p ../../../build_out/release_lin/lib 2>>$logFile 1>>$logFile
	cp lib/libwx_gtk2u_desura-2.9.so.0.0.0 ../../../build_out/release_lin/lib/libwx_gtk2u_desura-2.9.so.0 2>>$logFile 1>>$logFile
	
	if [ $? -ne 0 ]; then
		exitFailed "copy release"
	fi
	
	cd ..
}

function configureDebug()
{
	cd wxWidgets_debug

	./configure --enable-debug $CONFIG_OPTIONS 2>>$logFile 1>>$logFile
		
	if [ $? -ne 0 ]; then
		exitFailed "configure debug"
	fi
}

function makeDebug()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	
	if [ $? -ne 0 ]; then
		exitFailed "make debug"
	fi
	
	mkdir -p ../../../build_out/debug_lin/lib 2>>$logFile 1>>$logFile
	cp lib/libwx_gtk2ud_desura-2.9.so.0.0.0 ../../../build_out/debug_lin/lib/libwx_gtk2ud_desura-2.9.so.0 2>>$logFile 1>>$logFile
		
	if [ $? -ne 0 ]; then
		exitFailed "copy debug"
	fi

	cd ..
}

function main()
{
	echo > $logFile
	outputStarting
	
	if [ x"$1" == x"--clean" ]; then
		echo "           ( cleaning )"
		cd wxWidgets
		make clean 2>>$logFile 1>>$logFile
		cd ..
	
		rm -rf wxWidgets_debug 2>>$logFile 1>>$logFile
		exitSuccess
	fi
	
	patchWx
	copyToDebug
	
	configureRelease
	makeRelease

	configureDebug
	makeDebug
	
	exitSuccess	
}

# ENTRY POINT

main $@
