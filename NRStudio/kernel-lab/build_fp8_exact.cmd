@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" -vcvars_ver=14.38
if errorlevel 1 exit /b %errorlevel%
"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\nvcc.exe" -arch=sm_86 -O3 fp8_exact_test.cu -o fp8_exact_test.exe
exit /b %errorlevel%
