@echo on
cd ..\sqlite3x

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite3x.sln /t:rebuild /m /property:Configuration=Release
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite3x.sln /t:rebuild /m /property:Configuration=Release_Static
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" sqlite3x.sln /t:rebuild /m /property:Configuration=Debug

cd ..\build_win