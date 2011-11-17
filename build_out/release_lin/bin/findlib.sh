#!/bin/bash

BIT=""

if [ x"$2" == x"64" ]; then
	BIT="64"
	LIBDIRS="/lib64 /lib/x86_64-linux-gnu /lib /usr/lib64 /usr/lib/x86_64-linux-gnu /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu/nss /usr/lib/nss"
else
	BIT="32"
	LIBDIRS="/lib /lib/i386-linux-gnu /lib/i686-linux-gnu /lib32 /usr/lib /usr/lib32 /usr/lib/i686-linux-gnu /usr/lib/i386-linux-gnu /usr/local/lib /usr/lib/i386-linux-gnu/nss /usr/lib/i686-linux-gnu/nss /usr/lib/nss"
fi


for DIR in $LIBDIRS; do

	if [ -e "$DIR/$1" ]; then

		BUILD=`file -L "$DIR/$1" | grep 64-bit`

		#check to see if 64 bit lib
		if [ x"$BIT" == x"64" -a -z "$BUILD" ]; then
			continue;
		fi

		BUILD=`file -L "$DIR/$1" | grep 32-bit`

		#check to see if 32 bit lib
		if [ x"$BIT" == x"32" -a -z "$BUILD" ]; then
			continue;
		fi

		echo "$DIR/$1"
		exit 0
	fi
done

echo ""
exit -1
