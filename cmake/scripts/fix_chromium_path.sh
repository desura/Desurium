#!/bin/sh
if [ ! -d "src" ]; then
	cd ..
	mv chromium src
	mkdir chromium
	mv src chromium/
	cd chromium
fi
