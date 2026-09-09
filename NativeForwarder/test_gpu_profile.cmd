@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\external\nvapi" gpu_profile_tests.cpp /Fe:gpu_profile_tests.exe /link dxgi.lib
if errorlevel 1 exit /b %errorlevel%
gpu_profile_tests.exe
