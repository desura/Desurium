@echo on

"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" desura.sln /m /property:Configuration=Release

set BUILD_STATUS=%ERRORLEVEL% 
if %BUILD_STATUS%==0 goto next
if not %BUILD_STATUS%==0 goto fail 


:next
"C:\WINDOWS\Microsoft.NET\Framework\v4.0.30319\msbuild.exe" desura.sln /m /property:Configuration=Release_Static

set BUILD_STATUS=%ERRORLEVEL% 
if %BUILD_STATUS%==0 goto end 
if not %BUILD_STATUS%==0 goto fail 
 
:fail 
exit 1 
 
:end 
exit 0 