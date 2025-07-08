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

#include "src/finder.cpp"

#define INIT_TIMER                       \
  LARGE_INTEGER frequency;               \
  LARGE_INTEGER start, end;              \
  QueryPerformanceFrequency(&frequency); \

#define START_TIMER QueryPerformanceCounter(&start);

#define STOP_TIMER                                                              \
  QueryPerformanceCounter(&end);                                                \
  f64 elapsed_time = (f32)(end.QuadPart - start.QuadPart) / frequency.QuadPart; \

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
      case WM_PAINT:
      {
        // Render(hwnd);
        return 0;
      }
      case WM_KEYDOWN:
      case WM_DESTROY:
      {
        // Cleanup();
        PostQuitMessage(0);
        return 0;
      }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

s32 main(s32 argc, wchar* argv[])
{
  wchar class_name[] = L"win_finder";

  WNDCLASS window_class = {};
  window_class.lpfnWndProc   = WndProc;
  window_class.lpszClassName = class_name;
  window_class.hCursor       = LoadCursor(null, IDC_ARROW);
  RegisterClass(&window_class);

  s32 screen_width  = GetSystemMetrics(SM_CXSCREEN);
  s32 screen_height = GetSystemMetrics(SM_CYSCREEN);
  s32 window_width  = 1280;
  s32 window_height = 720;
  s32 window_x = (screen_width  / 2) - (window_width  / 2);
  s32 window_y = (screen_height / 2) - (window_height / 2);

  HWND window = CreateWindow(
    class_name, L"Win Finder",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    window_x, window_y, window_width, window_height,
    0, 0, 0, 0
  );

  wcscpy(query, argv[2]);

  if(argv[3] != null && wcscmp(argv[3], L"fuzzy") == 0)
  {
    string_match_proc = simple_fuzzy_match;
  }

  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  s32 total_threads = sysinfo.dwNumberOfProcessors;

  HANDLE *threads = (HANDLE*)calloc(sizeof(HANDLE), total_threads);

  ID2D1Factory *pD2DFactory = null;
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

  IDWriteFactory *pDWriteFactory = null;
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&pDWriteFactory);

  ID2D1HwndRenderTarget *pRenderTarget = null;
  RECT rc;
  GetClientRect(window, &rc);
  pD2DFactory->CreateHwndRenderTarget
  (
    D2D1::RenderTargetProperties(),
    D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right, rc.bottom)),
    &pRenderTarget
  );

  IDWriteTextFormat *pTextFormat = null;
  pDWriteFactory->CreateTextFormat(L"", null, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f, L"", &pTextFormat);

  pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  ID2D1SolidColorBrush *pBlackBrush = null;
  pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);


  puts("Working...");
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


bool has_substring(wchar *haystack, wchar *needle)
{
  s64 needle_length = wcslen(needle);
  if(needle_length == 0) return true;

  s64 haystack_length = wcslen(haystack);
  if(haystack_length == 0) return false;

  if(needle_length > haystack_length) return false;

  s64 haystack_index = 0;
  s64 needle_index   = 0;

  while(haystack_index < haystack_length)
  {
    if(towlower(haystack[haystack_index]) == towlower(needle[needle_index]))
    {
      needle_index++;

      if(needle_index < needle_length)
      {
        haystack_index++;
      }
      else
      {
        return true;
      }
    }
    else
    {
      needle_index = 0;
      haystack_index++;
    }
  }

  return false;
}

bool simple_fuzzy_match(wchar *haystack, wchar *needle)
{
  while(*haystack != '\0' && *needle != '\0')
  {
    if(tolower(*haystack) == tolower(*needle))
    {
      needle++;
    }

    haystack++;
  }

  return *needle == '\0';
}