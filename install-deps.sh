#!/bin/sh
# Determine the operating system distribution that is being run and install the build dependencies for it.

if [ "$(whoami)" != "root" ]; then
	echo "Run this as root! sudo should do the trick."
	exit
fi

if [ -f /etc/debian_version ]; then
	apt-get install gcc git-core subversion m4 build-essential binutils automake autoconf libtool libgtk2.0-dev libnss3-dev libgconf2-dev libgnome-keyring-dev libdbus-glib-1-dev gperf bison libcups2-dev flex libjpeg-dev libasound2-dev libbz2-dev libxpm-dev libx11-dev libssl-dev libnotify-dev scons libv8-dev libc-ares-dev libboost-dev libboost-date-time-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev
elif [ -f /etc/redhat-release ]; then
	yum install git subversion m4 autoconf gcc-c++ libstdc++-static glibc-devel binutils autoconf libtool gtk2-devel nss-devel GConf2-devel libgnome-keyring-devel dbus-glib-devel gperf bison cups-devel flex libjpeg-turbo-devel alsa-lib-devel bzip2-devel libXpm-devel libX11-devel openssl-devel libnotify-devel scons
elif [ -f /etc/arch-release ]; then
	DEPS="$(pacman -T git subversion m4 autoconf gcc glibc binutils autoconf libtool gtk2 nss libgnome-keyring dbus-glib gperf bison cups flex libjpeg-turbo alsa-lib bzip2 libxpm libx11 openssl scons gconf libnotify boost c-ares v8 bzip2 tinyxml sqlite3 | sed -e 's/\n/ /g')"
	if [ "${DEPS}" == "" ]; then
		echo "Required dependencies are already installed."
	else
		pacman -S --asdeps ${DEPS}
	fi
else
	echo "Unsupported operating system."
fi
