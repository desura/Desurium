#!/bin/bash

target=curl
logFile=$PWD/$target.log
origDir=$PWD 	# should be third_party
#cd $target		# not needed in this script (curl)

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

function checkDownload()
{
	if [ ! -d curl-shared -o ! -d curl-static ]; then
		echo -e "\nCurl not downloaded. Checking out first.\n" 2>>$logFile 1>>$logFile
		./checkout_curl.sh 2>>$logFile 1>>$logFile
	fi
}

function configureCares()
{
	cd c-ares

	if [ ! -e configure ]; then
		find . -name Makefile.am | xargs sed -i 's/libcares\(.la\)/libcares_desura\1/g' 2>>$logFile 1>>$logFile
		./buildconf 2>>$logFile 1>>$logFile
		buildconfReturn=$?
		if [ $buildconfReturn -ne 0 ]; then
			exitFailed "buildconf cares"
		fi
	fi

	if [ ! -e Makefile ]; then
		./configure --without-librtmp --disable-debug --enable-shared=yes --enable-static=no --disable-curldebug --enable-symbol-hiding --enable-optimize=-O2 2>>$logFile 1>>$logFile
		configureReturn=$?
		if [ $configureReturn -ne 0 ]; then
			exitFailed "configure cares"
		fi
	fi
}

function makeCares()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	makeReturn=$?
	if [ $makeReturn -ne 0 ]; then
		exitFailed "make cares"
	fi

	if [ ! -e libcares_desura.pc ]; then
		ln -s libcares.pc libcares_desura.pc 2>>$logFile 1>>$logFile # to past curl configure test
		symlinkReturn=$?
		if [ $symlinkReturn -ne 0 ]; then
			exitFailed "symlink libcares.pc"
		fi
	fi
	
	cp -p .libs/libcares_desura.so.2.0.0 ../../../build_out/release_lin/lib/libcares_desura.so.2 2>>$logFile 1>>$logFile
	cp -p .libs/libcares_desura.so.2.0.0 ../../../build_out/debug_lin/lib/libcares_desura.so.2 2>>$logFile 1>>$logFile

	cd ..
}

function configureCurlShared()
{
	cd curl-shared

	if [ ! -e configure ]; then
		# doing a buildconf in curl wipes ares, so delete the link if it's there, and add it back after
		if [ -e ares ]; then
			rm ares
		fi

		find . -name Makefile.am | xargs sed -i 's/libcurl\(.la\)/libcurl_desura\1/g' 2>>$logFile 1>>$logFile
		# twice to remove any old suffixes, so they don't double up
		find . -name Makefile.m32 | xargs sed -i 's/cares_desura/cares/g' 2>>$logFile 1>>$logFile
		find . -name Makefile.m32 | xargs sed -i 's/cares/cares_desura/g' 2>>$logFile 1>>$logFile
		find . -name curl-confopts.m4 | xargs sed -i 's/cares_desura/cares/g' 2>>$logFile 1>>$logFile
		find . -name curl-confopts.m4 | xargs sed -i 's/cares/cares_desura/g' 2>>$logFile 1>>$logFile
		find . -name testcurl.pl | xargs sed -i 's/cares_desura/cares/g' 2>>$logFile 1>>$logFile
		find . -name testcurl.pl | xargs sed -i 's/cares/cares_desura/g' 2>>$logFile 1>>$logFile
		
		chmod +x buildconf 2>>$logFile 1>>$logFile
		./buildconf 2>>$logFile 1>>$logFile
		buildconfReturn=$?
		if [ $buildconfReturn -ne 0 ]; then
			exitFailed "buildconf curl shared"
		fi

		if [ ! -e ares ]; then
			ln -s ../c-ares ares
		fi
	fi

	if [ ! -e Makefile ]; then
		chmod +x configure
		./configure --without-librtmp --disable-ldap --disable-debug --disable-curldebug --without-zlib --disable-rtsp --disable-manual --enable-static=no --enable-shared=yes --disable-pop3 --disable-imap --disable-dict --disable-gopher --disable-verbose --disable-smtp --disable-telnet --disable-tftp --disable-file --without-libidn --without-gnutls --without-nss --without-cyassl --without-axtls --without-libssh2 --enable-hidden-symbols --enable-cookies --without-sspi --disable-manual --enable-optimize=-O2 --enable-ares 2>>$logFile 1>>$logFile
		configureReturn=$?
		if [ $configureReturn -ne 0 ]; then
			exitFailed "configure curl shared"
		fi
	fi
}

function makeCurlShared()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	makeReturn=$?
	if [ $makeReturn -ne 0 ]; then
		exitFailed "make curl shared"
	fi
	
	cp -p lib/.libs/libcurl_desura.so.4.2.0 ../../../build_out/release_lin/lib/libcurl_desura.so.4 2>>$logFile 1>>$logFile
	cp -p lib/.libs/libcurl_desura.so.4.2.0 ../../../build_out/debug_lin/lib/libcurl_desura.so.4 2>>$logFile 1>>$logFile

	cd ..
}

function configureCurlStatic()
{
	cd curl-static

	if [ ! -e configure ]; then
		chmod +x buildconf 2>>$logFile 1>>$logFile
		./buildconf 2>>$logFile 1>>$logFile
		buildconfReturn=$?
		if [ $buildconfReturn -ne 0 ]; then
			exitFailed "buildconf curl static"
		fi
	fi

	if [ ! -e Makefile ]; then
		chmod +x configure
		.//configure --without-librtmp --disable-ldap --disable-debug --disable-curldebug --without-zlib --disable-rtsp --disable-manual --enable-static=yes --enable-shared=no --disable-pop3 --disable-imap --disable-dict --disable-gopher --disable-verbose --disable-smtp --disable-telnet --disable-tftp --disable-file --without-libidn --without-gnutls --without-nss --without-cyassl --without-ssl --without-axtls --without-libssh2 --enable-hidden-symbols --enable-cookies --without-sspi --disable-manual --enable-optimize=-O2 2>>$logFile 1>>$logFile
		configureReturn=$?
		if [ $configureReturn -ne 0 ]; then
			exitFailed "configure curl static"
		fi
	fi
}

function makeCurlStatic()
{
	make -j $MAKE_CONCURRENCY 2>>$logFile 1>>$logFile
	makeReturn=$?
	if [ $makeReturn -ne 0 ]; then
		exitFailed "make curl static"
	fi

	cd ../
}

function main()
{
	echo > $logFile
	outputStarting

	if [ x"$1" == x"--clean" ]; then
		echo "           ( cleaning )"
		rm -rf c-ares curl-static curl-shared 2>>$logFile 1>>$logFile
		exitSuccess
	fi

	checkDownload

	mkdir -p ../../build_out/release_lin/lib/ 2>>$logFile 1>>$logFile
	mkdir -p ../../build_out/debug_lin/lib/ 2>>$logFile 1>>$logFile

	configureCares
	makeCares

	configureCurlShared
	makeCurlShared
	
	configureCurlStatic
	makeCurlStatic

	exitSuccess
}

# ENTRY POINT
main $@
