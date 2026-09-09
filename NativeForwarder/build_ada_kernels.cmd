@echo off
"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\ptxas.exe" -arch=sm_89 -O3 -maxrregcount=224 -v ..\Kernels\post-direct.ptx -o post-ada.cubin
if errorlevel 1 exit /b %errorlevel%
"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v13.3\bin\ptxas.exe" -arch=sm_89 -O3 -maxrregcount=240 -v ..\Kernels\swin8-direct.ptx -o swin8-ada.cubin
exit /b %errorlevel%
