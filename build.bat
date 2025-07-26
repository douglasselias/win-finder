@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /s /q build
mkdir build
echo * > build\.gitignore

cl /nologo /Z7 /Wall /WX /W4 /utf-8 /Fo.\build\ /wd4189 /wd4996 /wd4100 /wd4244 /wd4255 /wd5045 /wd4711 /wd4710 /wd4365 /wd4211 /wd4820 /wd4456 /wd4505 /wd4668 /wd4201 /wd4577 /wd4702 /wd5219 main.cpp /fsanitize=address /link /out:build\main.exe

if exist build\main.exe (
  build\main "C:\Users\Douglas\Code\win-finder" .cpp
  @REM build\main "C:\Program Files (x86)" .h
  @REM build\main "C:" .blob
)