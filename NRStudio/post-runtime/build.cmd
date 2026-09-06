@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
rc /nologo candidate.rc
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /LD /DUNICODE /D_UNICODE /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\external\nvapi" /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\include\detours" native_forwarder.cpp candidate.res /Fe:nvngx.dll_dlssnr.dll /link d3d12.lib dxgi.lib bcrypt.lib "C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\library\detours\detours.lib"
exit /b %errorlevel%
