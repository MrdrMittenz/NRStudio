@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\nvcc.exe" -cubin -arch=sm_86 -O3 clear_candidate.cu -o clear_candidate.cubin
if errorlevel 1 exit /b %errorlevel%
cl /nologo /EHsc /O2 /std:c++17 /I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\include" clear_bench.cpp /Fe:clear_bench.exe /link /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\lib\x64" cuda.lib
exit /b %errorlevel%
