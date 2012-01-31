#!/bin/sh
# Determine the Linux distribution that is being run and install the build dependencies for it.

PASS_MESSAGE="Please provide root password to install build dependecies"

if [ -f /etc/SuSE-release ]; then
	DISTRIBUTION="suse"
elif [ -f /etc/debian_version ]; then
	DISTRIBUTION="debian"
	if [ "$(whoami)" != 'root' ]; then
		echo $PASS_MESSAGE
	fi
	su -c 'apt-get install gcc-4.5 git-core subversion m4 build-essential binutils automake autoconf libtool libgtk2.0-dev libnss3-dev libgconf2-dev libgnome-keyring-dev libdbus-glib-1-dev gperf bison libcups2-dev flex libjpeg-dev libasound2-dev libbz2-dev libxpm-dev libx11-dev libssl-dev libnotify-dev scons'
elif [ -f /etc/redhat-release ]; then
	DISTRIBUTION="redhat"
	if [ "$(whoami)" != 'root' ]; then
		echo $PASS_MESSAGE
	fi
	su -c 'yum install git subversion m4 autoconf gcc-c++ glibc-devel binutils autoconf libtool gtk2-devel nss-devel GConf2-devel libgnome-keyring-devel dbus-glib-devel gperf bison cups-devel flex libjpeg-turbo-devel alsa-lib-devel bzip2-devel libXpm-devel libX11-devel openssl-devel libnotify-devel scons'
elif [ -f /etc/arch-release ]; then 
	DISTRIBUTION="arch"
	if [ "$(whoami)" != 'root' ]; then
		echo $PASS_MESSAGE
	fi
	su -c 'pacman -S git subversion m4 autoconf gcc glibc binutils autoconf libtool gtk2 nss  libgnome-keyring dbus-glib gperf bison cups flex libjpeg-turbo alsa-lib bzip2 libxpm libx11 openssl scons gconf libnotify'

fi
