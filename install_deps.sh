#!/bin/sh
# Determine the operating system distribution that is being run and install the build dependencies for it.

if [ "$(whoami)" != "root" ]; then
	echo "Run this as root! sudo should do the trick."
	exit
fi

if [ -f /etc/debian_version ]; then # Debian  (untested!)
	echo -e "\e[1;31mDebian detected!\e[0m"
	echo -e "\e[1;31mNote: you can build a DEB package running './build_desura.sh pack_deb'\e[0m"
	DEPS="autoconf automake binutils bison build-essential cmake flex gcc gperf libasound2-dev libboost-dev libboost-date-time-dev libboost-filesystem-dev libboost-system-dev libboost-test-dev libboost-thread-dev libbz2-dev libc-ares-dev libcups2-dev libdbus-glib-1-dev libevent-dev libflac-dev libgconf2-dev libgnome-keyring-dev libgtk2.0-dev libjpeg62-dev libnotify-dev libnss3-dev libpng12-dev libspeex-dev libsqlite3-dev libssl-dev libtinyxml-dev libtool libv8-dev libx11-dev libxml2-dev libxpm-dev libxslt1-dev m4 scons xdg-utils yasm libxt-dev"
	if [ `uname -m` = 'x86_64' ]; then # 64 bit dependencies for 32BIT_SUPPORT
		DEPS=$DEPS" libc6-dev-i386 gcc-multilib"
	fi
	ST=`dpkg -l ${DEPS} 2>/dev/null | grep "^ii\ "  | cut -d' ' -f2`
	for PACK in ${DEPS}
	do
		CHECK=`echo "${ST}" | grep ${PACK}`
		if [ "${CHECK}" = "" ] ; then
			MUST_BE_INSTALLED="${MUST_BE_INSTALLED} ${PACK}"
		fi
	done
	if [ "${MUST_BE_INSTALLED}" != "" ] ; then
		apt-get install ${MUST_BE_INSTALLED}
	else
		echo "everything installed"
	fi
elif [ -f /etc/redhat-release ]; then # Red Hat
	echo -e "\e[1;31mRed Hat detected!\e[0m"
	echo -e "\e[1;31mNote: you can build a RPM package running './build_desura.sh pack_rpm'\e[0m"
	yum install m4 autoconf gcc-c++ libstdc++-static glibc-devel binutils autoconf libtool gtk2-devel nss-devel GConf2-devel libgnome-keyring-devel dbus-glib-devel gperf bison cups-devel flex libjpeg-turbo-devel alsa-lib-devel bzip2-devel libXpm-devel libX11-devel openssl-devel libnotify-devel scons xdg-user-dirs v8-devel c-ares-devel sqlite-devel libxslt-devel yasm-devel libevent-devel boost-devel boost-static patch perl-Digest-MD5 libXt-devel
elif [ -f /etc/arch-release ]; then		# Arch Linux
	echo -e "\e[1;31mArch Linux detected!\e[0m"
	echo -e "\e[1;31mNote: there are PKGBUILDs in ./distro/archlinux/\e[0m"
	# By using "pacman -T" to find out needed dependencies, we don't get
	# conflicts if a package we have installed provides one of the dependencies.
	DEPS="cmake make boost glib2 pkg-config sqlite m4 autoconf gcc glibc autoconf libtool gtk2 nss libgnome-keyring dbus-glib gperf bison cups flex libjpeg-turbo alsa-lib bzip2 libxpm libx11 openssl scons gconf libnotify xdg-user-dirs v8 c-ares sed flac libpng speex zlib xdg-utils libevent libxslt yasm libxml2 libxxf86vm flashplugin libx11 libxt"
	arch=`uname -m`

	if [ "${arch}" == "x86_64" ] ; then
		DEPS="${DEPS} binutils-multilib"
	elif [ "${arch}" == "i386" ] ; then # no extra-32-bit support needed
		DEPS="${DEPS} binutils"
	fi
	DEPS=`pacman -T ${DEPS} | sed -e 's/\n/ /g'`

	if [ -z "${DEPS}" ]; then
		echo "Dependencies already installed."
	else
		pacman -S --asdeps ${DEPS}
	fi
elif [ -f /usr/bin/emerge ]; then # Gentoo
	echo "Please copy the ebuild under gentoo/portage into a local overlay and emerge via emerge --onlydeps desura"
else # else
	echo "Unsupported operating system."
fi
