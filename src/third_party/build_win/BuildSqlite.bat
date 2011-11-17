@echo on
cd ..\sqlite

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite.sln /t:rebuild /m /property:Configuration=Release
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite.sln /t:rebuild /m /property:Configuration=Release_Static
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite.sln /t:rebuild /m /property:Configuration=Debug

cd ..\build_win