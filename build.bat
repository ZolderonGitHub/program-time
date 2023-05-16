@echo off

REM replace this with your current install of visual studio path
set VCVARSALL_BULLSHIT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"

call %VCVARSALL_BULLSHIT% x64 > nul

REM build command
IF NOT EXIST build mkdir build
pushd build
cl -WL -W4 -wd4295 -Od -Fe"program_time" ../code/program_time.c
popd

