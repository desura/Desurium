#!/bin/sh
printf 'Make sure to run \033[1;31msudo ./install-deps.sh\033[0m before compiling!\n\n'

initial_dir=`pwd`
PREFIX="../install"
BINDIR=""
LIBDIR="lib"
DATADIR=""


compile_cef() {
	echo "Compiling cef..."
	if [ ! -d "build_cef" ] ; then
		mkdir build_cef
	fi
	cd build_cef
	cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DRUNTIME_LIBDIR=$LIBDIR -DBUILD_ONLY_CEF=ON || exit 1
	make install $args
	cd ../
	printf "\n"
}

compile_desurium() {
	echo "Compiling desurium..."
	if [ ! -d "build" ] ; then
		mkdir build
	fi
	cd build
	cmake .. -DCMAKE_INSTALL_PREFIX=$PREFIX -DBINDIR=$BINDIR -DRUNTIME_LIBDIR=$LIBDIR -DDATADIR=$DATADIR -DBUILD_CEF=OFF || exit 3
	make install $args
}

clean_cef() {
	echo "Making clean for cef..."
	if [ -d "build_cef" ] ; then
		cd ./build_cef/
		make clean
		cd ../
	fi
	if [ ! -d "build" ] ; then
		echo "Removing install directory..."
		rm -rf "install"
	fi
	echo "Done"
}

clean_desurium() {
	echo "Making clean for desurium..."
	if [ -d "build" ] ; then
		cd ./build/
		make clean
		cd ../
	fi
	if [ ! -d "build_cef" ] ; then
		echo "Removing install directory..."
		rm -rf "install"
	fi
	printf "Done\n"
}


case "$@" in
	"help" )
		echo "\"desurium\" refers to the actual client."
		echo "\"cef\" is Chormium Embedded Framework, a runtime dependency."
		echo "\"all\" refers to both."
		echo ""
		echo "\"rebuild\" performs make clean and rebuild the respective targets."
		echo "\"compile\" compiles the respective target."
		echo "\"clean\" performs make clean for the respective target."
		echo ""
		echo "\"check\" performs make check for desurium."
		echo ""
		echo "\"help\" displays this text."
		echo ""
		echo "Everything but \"help\" and the \"clean\" commands accept arguments."
		echo "Example:      ./build_desura.sh rebuild_all -j `getconf _NPROCESSORS_ONLN`"
		finished="nope"
		;;
	*rebuild_all* )
		args=`echo "$@" | sed -e 's/rebuild_all//'`
		clean_cef
		clean_desurium
		compile_cef
		compile_desurium
		;;
	*rebuild_cef* )
		args=`echo "$@" | sed -e 's/rebuild_cef//'`
		clean_cef
		compile_cef || exit 1
		;;
	*rebuild_desurium* )
		args=`echo "$@" | sed -e 's/rebuild_desurium//'`
		clean_desurium
		compile_desurium || exit 2
		;;
	*compile_desurium* )
		args=`echo "$@" | sed -e 's/compile_desurium//'`
		compile_desurium || exit 3
		;;
	*compile_cef* )
		args=`echo "$@" | sed -e 's/compile_cef//'`
		compile_cef || exit 4
		;;
	"clean_cef" )
		clean_cef
		exit
		;;
	"clean_desurium" )
		clean_desurium
		exit
		;;
	"clean_all" )
		clean_cef
		clean_desurium
		exit
		;;
	*check* )
		echo "'make check' will be called."
		args=`echo "$@" | sed -e 's/check//'`
		compile_cef
		compile_desurium
		printf "Running 'make check'\n"
		# already in /build
		make test
		;;
	* )
		args=$@
		compile_cef
		compile_desurium
		;;
esac

cd "${initial_dir}"

if [ -d "build" ] && [ -d "build_cef" ] && [ -d "install" ] && [ -z $finished ] ; then
	printf 'Run \033[1;31m./install/desura\033[0m to start Desura!\n'
elif [ -z $finished ] ; then
	echo ""
	echo "In order to run Desura, we need cef as well as desurium compiled."
	echo "Please re-run the script to automatically compile both."
fi
