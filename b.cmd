@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
set VSLANG=1033
cd /d D:\git\deskflow\build
cmake --build . --target deskflow-core --config Release -j1 2>D:\git\deskflow\err.log
echo EXIT:%ERRORLEVEL%
