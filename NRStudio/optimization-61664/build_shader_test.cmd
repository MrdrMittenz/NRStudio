@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%
cl /nologo /std:c++17 /EHsc /O2 shader_bench.cpp /Fe:shader_bench.exe /link d3d12.lib d3dcompiler.lib dxgi.lib
exit /b %errorlevel%
