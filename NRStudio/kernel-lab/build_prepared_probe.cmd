@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\nvcc.exe" -cubin -arch=sm_86 -O3 prepare_weights.cu -o prepare_weights.cubin
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\external\nvapi" /I"C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\include\detours" prepared_probe.cpp /Fe:prepared_probe.exe /link d3d12.lib dxgi.lib "C:\Users\ReviOSGaming\AI-Work\optiscaler-nr-reference\OptiScaler\library\detours\detours.lib"
exit /b %errorlevel%
