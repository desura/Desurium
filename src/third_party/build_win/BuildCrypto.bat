@echo on
cd ..\crypto

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" crypto.sln /t:rebuild /m /property:Configuration=Release
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" crypto.sln /t:rebuild /m /property:Configuration=Debug

cd ..\build_win