@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
cl /nologo /EHsc /O2 /LD cpu_forwarder.cpp /Fe:nvngx.dll_dlssnr.dll
exit /b %errorlevel%
