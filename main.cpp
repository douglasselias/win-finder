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

typedef int64_t s64;
typedef int32_t s32;
typedef float   f32;
typedef double  f64;
typedef wchar_t wchar;

#define null NULL

#include "src/string_matcher.cpp"
#include "src/finder.cpp"
#include "src/ui.cpp"
#include "src/timer.cpp"

s32 main(s32 argc, char* argv[])
{
  create_window();
  
  mbstowcs(dir,   argv[1], strlen(argv[1]));
  mbstowcs(query, argv[2], strlen(argv[2]));

  if(argv[3] != null && strcmp(argv[3], "fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

  create_threads();

  MSG msg;
  while(GetMessage(&msg, null, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;

#if 0
  puts("Searching...\n");

  INIT_TIMER
  START_TIMER

  list_files_from_directory(argv[1]);

  for(s32 i = 0; i < total_threads; i++)
  {
    threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
  }

  WaitForMultipleObjects(total_threads, threads, true, INFINITE);

  STOP_TIMER

  printf("\n Scanned %lld files and found %lld files in %.2f seconds.\n", total_files_scanned, total_files_found, elapsed_time);
#endif
}