#define UNICODE

#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <d2d1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

typedef int64_t  s64;
typedef int32_t  s32;
typedef uint64_t u64;
typedef float    f32;
typedef double   f64;
typedef wchar_t  wchar;

#define null NULL

HWND list;
HWND window;

#include "src/string_matcher.cpp"
#include "src/finder.cpp"
#include "src/ui.cpp"
#include "src/timer.cpp"

s32 main(s32 argc, wchar* argv[])
{
  create_window();

  wcscpy(dir,   argv[1]);
  wcscpy(query, argv[2]);

  if(argc > 3 && wcscmp(argv[3], L"fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

  MSG msg;
  while(GetMessage(&msg, null, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}