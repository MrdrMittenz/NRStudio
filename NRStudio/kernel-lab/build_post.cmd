@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
cl /nologo /EHsc /O2 /std:c++17 /I"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\include" post_bench.cpp /Fe:post_bench.exe /link /LIBPATH:"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\lib\x64" cuda.lib
exit /b %errorlevel%
