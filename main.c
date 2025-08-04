#define UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

typedef   signed int       s32;
typedef   signed long long s64;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef float  f32;
typedef double f64;

typedef wchar_t wchar;
#define null    NULL

HWND window;
HWND list;
HWND labelScanned;
HWND labelFound;

wchar dir[MAX_PATH] = {0};

#include "src/string_matcher.c"
#include "src/finder.c"
#include "src/timer.c"
#include "src/ui.c"

s32 main(s32 argc, char *argv[])
{
  string_match_proc = simple_fuzzy_match;

  create_threads();
  
  create_window();

  MSG msg;
  while(GetMessage(&msg, null, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}