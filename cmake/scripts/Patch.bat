@echo off
patch -p0 -N -i %1

REM if we hit one means the patch is all ready applied
IF ["%errorlevel%"]==["1"] (
	EXIT /B 0
)
