@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\external\nvapi" /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\include\detours" direct_probe.cpp /Fe:direct_probe.exe /link d3d12.lib dxgi.lib bcrypt.lib "C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\library\detours\detours.lib"
exit /b %errorlevel%
