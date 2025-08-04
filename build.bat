@echo off

cls
where /q cl || call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

rmdir /S /Q .\build
mkdir build

pushd .\build

echo * > .gitignore

cl /nologo /fsanitize=address /Z7 /diagnostics:caret /Wall /WX /W4 /wd4668 /wd4996 /wd4100 /wd4255 /wd5045 /wd4711 /wd4710 /wd4189 /wd4820 ..\main.c

popd

if exist build\main.exe (
  @REM build\main.exe C: raddbg
  build\main.exe C: .cpp
  @REM build\main.exe C:\Users\Douglas\Code\win-finder .c
)