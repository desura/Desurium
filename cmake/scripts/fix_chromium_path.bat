@echo off
IF NOT EXIST chromium/src (
	move chromium src
	md chromium
	move src chromium/src
)