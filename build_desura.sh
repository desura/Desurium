#!/bin/sh
printf 'Make sure to run \033[1;31msudo ./install_deps.sh\033[0m before compiling!\n\n'

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
	if [ -d "build" ] ; then # desurium has been build
		find install/ -type f -iname libcef_desura.so -delete # remove only cef lib
	else # desurium has not been build, there are only cef related files in /install
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
	if [  -d "build_cef" ] ; then # cef has been build
		find install/ -type f ! -iname libcef_desura.so -delete
	else # cef has not been build, there are only desurium related files in /install
		echo "Removing install directory..."
		rm -rf "install"
	fi
	printf "Done\n"
}

pack() {
	finished="nope"
	echo "Building "$PACKAGE" package..."
	if [ ! -d "build_package" ] ; then
		mkdir build_package
	fi
	cd build_package
	cmake .. -DPACKAGE_TYPE=$PACKAGE -DINSTALL_DESKTOP_FILE=ON -DCMAKE_INSTALL_PREFIX="/opt/desura" || exit
	make package $args
	if [ $PACKAGE = "DEB" ]; then
		mv Desura-*.deb ..
	elif [ $PACKAGE = "RPM" ]; then
		mv Desura-*.rpm ..
	fi
	cd ..
	rm -rf build_package
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
		echo "\"pack_deb\" performs building DEB package of desurium."
		echo "\"pack_rpm\" performs building RPM package of desurium."
		echo ""
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
	*pack_deb* )
		args=`echo "$@" | sed -e 's/pack_deb//'`
		PACKAGE="DEB"
		pack || exit 5
		;;
	*pack_rpm* )
		args=`echo "$@" | sed -e 's/pack_rpm//'`
		PACKAGE="RPM"
		pack || exit 6
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
		compile_cef || exit 1
		compile_desurium || exit 2
		printf "Running 'make check'\n"
		# already in /build
		make test
		;;
	* )
		args=$@
		compile_cef || exit 1
		compile_desurium || exit 2
		;;
esac

cd "${initial_dir}"


if [ -f "install/lib/desura" ] ; then #desura has been build
	if [ -f "install/lib/libcef_desura.so" ] ; then # cef has been build
		if [ -z ${finished} ] ; then # we did not call "help"
			printf 'Run \033[1;31m./install/desura\033[0m to start Desura!\n'
		fi
	else # cef not build
		echo "Please make sure to have cef compiled (run './build_desura.sh compile_cef') if you want to run Desura."
	fi
else # desurium not build
	echo "Please make sure to have desurium compiled (run './build_desura.sh compile_desurium') if you want to run Desura."
fi
