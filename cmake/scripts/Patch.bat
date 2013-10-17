@echo off
REM try a revert first
"@PATCH_EXECUTABLE@" -R --dry-run -p0 -N -i %1
if errorlevel 1 (
  "@PATCH_EXECUTABLE@" -p0 -N -i %1
)
REM else we are lucky, patch was already applied and we can ignore everything
