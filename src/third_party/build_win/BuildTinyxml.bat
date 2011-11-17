@echo on
cd ..\tinyxml

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" tinyxml.sln /t:rebuild /m /property:Configuration=Release
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" tinyxml.sln /t:rebuild /m /property:Configuration=Release_Static
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" tinyxml.sln /t:rebuild /m /property:Configuration=Debug

cd ..\build_win