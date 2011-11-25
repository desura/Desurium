@echo off

cls

echo _
echo _
echo    MAKE SURE YOU PATCH breakpad FIRST
echo _
echo    Copy wxWidgets_patch.patch from patches folder into the wxWidgets folder.
echo    Right click on the patch file and select tortisesvn - apply patch
echo    Highlight the files and right click - patch files
echo _
echo _

@echo on

pause

cd ..\wxWidgets\build\msw

if [%1]==[--clean] (
	nmake /nologo -f makefile.vc BUILD=release clean
	nmake /nologo -f makefile.vc BUILD=debug clean
)

nmake /nologo -f makefile.vc BUILD=release MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1
nmake /nologo -f makefile.vc BUILD=debug MONOLITHIC=1 VENDOR=desura DEBUG_INFO=1 SHARED=1
nmake /nologo -f makefile.vc BUILD=release VENDOR=desura DEBUG_INFO=1 SHARED=0 RUNTIME_LIBS=static
nmake /nologo -f makefile.vc BUILD=debug VENDOR=desura DEBUG_INFO=1 SHARED=0

cd ..\..

mkdir ..\..\..\build_out\release_win\bin
mkdir ..\..\..\build_out\debug_win\bin

copy lib\vc_dll\wxmsw290u_vc_desura.dll ..\..\..\build_out\release_win\bin\wxmsw290u_vc_desura.dll
copy lib\vc_dll\wxmsw290ud_vc_desura.dll ..\..\..\build_out\debug_win\bin\wxmsw290ud_vc_desura.dll

cd ..\build_win