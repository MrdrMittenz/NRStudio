@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%
msbuild OptiScaler.sln /t:OptiScaler /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 /p:PostBuildEventUseInBuild=false /m:4 /v:minimal > capture-build.log 2>&1
exit /b %errorlevel%
