@echo off

cls

echo _
echo _
echo    MAKE SURE YOU PATCH breakpad FIRST
echo _
echo    Copy breakpad_patch.patch from patches folder into the breakpad folder.
echo    Right click on the patch file  and select tortisesvn - apply patch
echo    Highlight the files and right click - patch files
echo _
echo _

@echo on

pause

cd ..\breakpad\src\client\windows
cmd /C python ..\..\tools\gyp\gyp breakpad_client.gyp

IF NOT EXIST build_all.vcxproj devenv /upgrade breakpad_client.sln

msbuild.exe handler\exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=Release
msbuild.exe handler\exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=Debug
msbuild.exe handler\exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=Release_Static

cd ..\..\..\..\build_win