ID2D1HwndRenderTarget *render_target = null;
IDWriteTextFormat *text_format = null;
ID2D1SolidColorBrush *brush = null;

s32 total_threads = 1;
HANDLE *threads = null;

void create_threads()
{
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  total_threads = sysinfo.dwNumberOfProcessors;

  threads = (HANDLE*)calloc(sizeof(HANDLE), total_threads);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
  switch(msg)
  {
    // case WM_PAINT:
    // {
    //   // render_target->BeginDraw();
    //   // render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));

    //   // RECT rc;
    //   // GetClientRect(hwnd, &rc);

    //   // // wchar buffer[MAX_PATH];
    //   // // DWORD length = GetCurrentDirectory(MAX_PATH, buffer);
    //   // // render_target->DrawText(buffer, (u32)length, text_format, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), brush);

    //   // // wchar *text = L"电视机! Good work! 😎 نطقها نطقًا";
    //   // wchar *text = results[0];
    //   // render_target->DrawText(text, (UINT32)wcslen(text), text_format, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), brush);

    //   // render_target->EndDraw();
    //   return 0;
    // }
    case WM_DESTROY:
    {
      // Cleanup();
      PostQuitMessage(0);
      return 0;
    }
    case WM_CREATE:
    {
      list = CreateWindow(L"LISTBOX", null,  WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 10, 10, 300, 600, hwnd, null, GetModuleHandle(null), null);
      break;
    }
    case WM_KEYDOWN:
    {
      switch(w_param)
      {
        case VK_ESCAPE:
        case VK_OEM_3:
        {
          PostQuitMessage(0);
          return 0;
        }
        case VK_SPACE:
        {
          list_files_from_directory(dir);

          for(s32 i = 0; i < total_threads; i++)
          {
            threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
          }

          WaitForMultipleObjects(total_threads, threads, true, INFINITE);

          SendMessage(hwnd, WM_PAINT, 0, 0);

          for(s64 i = 0; i < 4; i++)
          {
            wprintf(L"Result: %lld, %s\n", i, work_to_do[i]);
          }
          break;
        }
      }
      break;
    }
  }

  return DefWindowProc(hwnd, msg, w_param, l_param);
}

void create_window()
{
   wchar class_name[] = L"win_finder";

  WNDCLASS window_class = {};
  window_class.lpfnWndProc   = window_proc;
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

      // ShowWindow(window, SW_SHOW);
    // UpdateWindow(window);

  /// D2D Init ///
  
  ID2D1Factory *d2d_factory = null;
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);

  IDWriteFactory *direct_write_factory = null;
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&direct_write_factory);

  // ID2D1HwndRenderTarget *render_target = null;
  RECT rc;
  GetClientRect(window, &rc);
  d2d_factory->CreateHwndRenderTarget
  (
    D2D1::RenderTargetProperties(),
    D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right, rc.bottom)),
    &render_target
  );

  // IDWriteTextFormat *text_format = null;
  direct_write_factory->CreateTextFormat(L"", null, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f, L"", &text_format);

  text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  // ID2D1SolidColorBrush *brush = null;
  render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);
}