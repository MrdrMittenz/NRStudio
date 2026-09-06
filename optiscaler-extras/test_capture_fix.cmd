@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 capture_readback_test.cpp /Fe:capture_readback_test.exe /link d3d12.lib dxgi.lib
if errorlevel 1 exit /b %errorlevel%
capture_readback_test.exe
exit /b %errorlevel%
