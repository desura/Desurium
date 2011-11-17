@echo off

cls

echo _
echo _
echo MAKE SURE YOU PATCH v8 FIRST
echo MAKE SURE YOU have python 2.7 installed to C:\Python27
echo MAKE SURE YOU have scrons installed
echo _
echo _


@echo on

pause

setlocal enabledelayedexpansion

if NOT [%V8_BUILD_SETPATH%]==[v8_set] (
	echo setting path
	set PATH=!PATH!;C:\Python27;C:\Python27\Scripts
)

set V8_BUILD_SETPATH=v8_set

cmd /C "v8_make.bat"
cmd /C "v8_make.bat debug"
cmd /C "v8_make.bat static"