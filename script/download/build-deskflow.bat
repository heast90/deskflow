@echo off
setlocal enabledelayedexpansion

echo === Deskflow Build ===
echo.

call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat"

set BUILD_DIR=D:\git\deskflow\build

echo Building deskflow-core...
cd /d "%BUILD_DIR%"
cmake --build . --target deskflow-core 1>D:\git\deskflow\script\download\build-deskflow.txt 2>&1

echo.
echo === Exit code: %ERRORLEVEL% ===
if "%ERRORLEVEL%"=="0" (
  echo Build successful!
  echo Output: %BUILD_DIR%\bin\deskflow-core.exe
  dir "%BUILD_DIR%\bin\deskflow-core.exe" 2>nul
) else (
  echo Build FAILED!
  echo Check: D:\git\deskflow\script\download\build-deskflow.txt
  pause
)
