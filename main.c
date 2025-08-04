#define UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#pragma comment(lib, "user32.lib")

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
wchar dir[MAX_PATH] = {0};

#include "src/string_matcher.c"
#include "src/finder.c"
#include "src/timer.c"
#include "src/ui.c"

// HANDLE *create_threads()
// {
//   SYSTEM_INFO sysinfo;
//   GetSystemInfo(&sysinfo);
//   u32 threads_count = sysinfo.dwNumberOfProcessors;

//   u32 total_threads = threads_count - 1;
//   HANDLE *threads = calloc(sizeof(HANDLE), total_threads);
//   return threads;
// }

s32 main(s32 argc, char *argv[])
{
  mbstowcs(dir,   argv[1], strlen(argv[1]));
  mbstowcs(query, argv[2], strlen(argv[2]));

  // printf("Current directory: %ls\n", dir);
  // printf("Query            : %ls\n", query);

  if(argv[3] != null && strcmp(argv[3], "fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

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