#!/bin/sh
CHECK=`cat gclient | grep "exec python2"`
if [ "$CHECK" = "" ]
	then find . -type f -exec sed -i -r -e "s/exec python/exec python2/" {} +
fi
