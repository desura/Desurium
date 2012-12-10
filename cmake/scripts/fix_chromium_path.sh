#!/bin/sh
if [ ! -d "chromium/src" ]; then
	mv chromium src
	mkdir chromium
	mv src chromium/
fi
