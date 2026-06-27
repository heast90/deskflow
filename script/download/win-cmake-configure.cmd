@echo off

call "C:\Program Files\Microsoft Visual Studio\18\Enterprise\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

if errorlevel 1 exit /b 1

cmake -S D:\git\deskflow -B D:\git\deskflow\build ^
  -G Ninja ^
  -DCMAKE_BUILD_TYPE=Release ^
  -DBUILD_TESTS=OFF ^
  -DBUILD_GUI=OFF ^
  -DCMAKE_PREFIX_PATH=D:\git\Qt\6.8.3\msvc2022_64 ^
  -DOPENSSL_ROOT_DIR=D:\git\OpenSSL-Build ^
  2>&1

exit /b %ERRORLEVEL%
