#!/bin/sh
# Determine the operating system distribution that is being run and install the build dependencies for it.

if [ "$(whoami)" != "root" ]; then
	echo "Run this as root! sudo should do the trick."
	exit
fi

if [ -f /etc/debian_version ]; then # Debian  (untested!)
    DEPS="gcc git-core subversion m4 build-essential binutils automake autoconf libtool libgtk2.0-dev libnss3-dev libgconf2-dev libgnome-keyring-dev libdbus-glib-1-dev gperf bison libcups2-dev flex libasound2-dev libbz2-dev libxpm-dev libx11-dev libssl-dev libnotify-dev scons libv8-dev libc-ares-dev libboost-dev libboost-date-time-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev"
    ST=`dpkg -l ${DEPS} | awk /^ii\ /  | awk '{print $2}'`
    for PACK in ${DEPS}
    do
        CHECK=`echo "${ST}" | grep ${PACK}`
        if [ "${CHECK}" = "" ] ; then
            MUST_BE_INSTALLED="${MUST_BE_INSTALLED} ${PACK}"
        fi
    done
    if [ "${MUST_BE_INSTALLED}" != "" ] ; then
        aptitude install ${MUST_BE_INSTALLED}
    else
        echo "everything intalled"
    fi
elif [ -f /etc/redhat-release ]; then
	yum install git subversion m4 autoconf gcc-c++ libstdc++-static glibc-devel binutils autoconf libtool gtk2-devel nss-devel GConf2-devel libgnome-keyring-devel dbus-glib-devel gperf bison cups-devel flex libjpeg-turbo-devel alsa-lib-devel bzip2-devel libXpm-devel libX11-devel openssl-devel libnotify-devel scons xdg-user-dirs v8-devel c-ares-devel
elif [ -f /etc/arch-release ]; then
	echo -e "\e[1;31mArch Linux detected!\e[0m"
	echo -e "\e[1;31mNote: there is a pkgbuild in ./distro/archlinux/\e[0m"
	# By using "pacman -T" to find out needed dependencies, we don't get
	# conflicts if a package we have installed provides one of the dependencies.
	DEPS=`pacman -T sqlite git subversion m4 autoconf gcc glibc binutils autoconf libtool gtk2 nss libgnome-keyring dbus-glib gperf bison cups flex libjpeg-turbo alsa-lib bzip2 libxpm libx11 openssl scons gconf libnotify xdg-user-dirs v8 c-ares sed | sed -e 's/\n/ /g'`
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
