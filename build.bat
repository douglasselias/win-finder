@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /S /Q .\build
mkdir build

cl /nologo /O2 /Wall /WX /W4 /Fo.\build\ /wd4189 /wd4996 /wd4100 /wd4244 /wd4255 /wd5045 /wd4711 /wd4710 /wd4365 /wd4211 /wd4820 /wd4456 /wd4505 /wd4668 /wd4201 /wd4577 main.cpp /link /out:build\main.exe

build\main "C:\Program Files (x86)" .h
@REM build\main "C:" .blob