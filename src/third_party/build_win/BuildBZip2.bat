@echo on
cd ..\bzip2

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" bzip2.sln /t:rebuild /m /property:Configuration=Release_Static
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" bzip2.sln /t:rebuild /m /property:Configuration=Release
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" bzip2.sln /t:rebuild /m /property:Configuration=Debug

mkdir lib
copy build\Debug\* lib\.
copy build\Release\* lib\.
copy build\Release_Static\* lib\.

cd ..\build_win