"@PATCH_EXECUTABLE@" -p0 -N -i %1
REM if we hit one means the patch is all ready applied
IF ["%errorlevel%"]==["1"] (
	SET errorlevel=0
)