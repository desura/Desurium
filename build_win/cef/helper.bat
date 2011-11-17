
@ECHO OFF

echo ----------------------------------------------
echo Build Path: %1
echo ----------------------------------------------

echo Copying cef_desura
copy %CEF_PATH%\%1\avcodec-53.dll ..\..\..\build\win_%1\bin\avcodec-53.dll
copy %CEF_PATH%\%1\avformat-53.dll ..\..\..\build\win_%1\bin\avformat-53.dll
copy %CEF_PATH%\%1\avutil-51.dll ..\..\..\build\win_%1\bin\avutil-51.dll
copy %CEF_PATH%\%1\icudt.dll ..\..\..\build\win_%1\bin\icudt.dll
copy %CEF_PATH%\%1\cef_desura.dll ..\..\..\build\win_%1\bin\cef_desura.dll

echo ----------------------------------------------


if [%1]==[Release] (
	echo Adding symbols to symbol store.
	echo This could take a while ......
	symstore add /r /f %CEF_PATH%\%1\*.* /s J:\Symbols /t "CEF" /c "%datetime%"
	
	echo Adding symbols to crash server.
	echo This could take a while ......	
	cd ..\..\tools\crashdumps\
	SymAutoUploader.exe %CEF_PATH%\%1\
)