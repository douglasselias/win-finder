// ID2D1HwndRenderTarget *render_target = null;
// IDWriteTextFormat *text_format = null;
// ID2D1SolidColorBrush *brush = null;

s32 total_threads = 1;
HANDLE *threads = null;
s32 window_width  = 600;
s32 window_height = 600;
HFONT hFont = null;

wchar buffer_scanned[64] = {0};
#define buffer_scanned_format L"Total files scanned: %lld"

wchar buffer_found[64] = {0};
#define buffer_found_format L"Total files found: %lld"

void create_threads()
{
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  total_threads = system_info.dwNumberOfProcessors;

  threads = (HANDLE*)calloc(sizeof(HANDLE), total_threads);
}

void repaint_window(HWND hwnd)
{
  InvalidateRect(hwnd, null, true);
  UpdateWindow(hwnd);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
static HWND editDir = NULL;  // Edit control for directory
static HWND editQuery = NULL; // Edit control for query
static HWND btnSearch = NULL; // Search button
static HWND labelDir = NULL;  // Label for directory
static HWND labelQuery = NULL; // Label for query
static HBRUSH hBrush = NULL;

// HWND labelScanned = NULL; // Label for total files scanned
// HWND labelFound = NULL;   // Label for total files found

    switch(msg)
    {
      case WM_CREATE:
      {
hBrush = CreateSolidBrush(RGB(255, 255, 255)); // White color

RECT clientRect;
      GetClientRect(hwnd, &clientRect);
      s32 clientWidth = clientRect.right - clientRect.left;
      s32 labelWidth = 80;
      s32 inputWidth = 420;
      s32 buttonWidth = 90;
      s32 buttonHeight = 30;
      s32 counterWidth = 200; // Width for scanned/found labels
      s32 totalInputWidth = labelWidth + inputWidth;
      s32 inputX = (clientWidth - totalInputWidth) / 2; // Center inputs and labels
      s32 buttonX = (clientWidth - buttonWidth) / 2;    // Center button
      s32 counterX = inputX + totalInputWidth + 10; // Right of inputs

        list = CreateWindow(L"LISTBOX", null,  WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER, 10, 120, window_width - (10 * 4), window_height - (10 * 16), hwnd, null, GetModuleHandle(null), null);
        // SendMessage(list, LB_INITSTORAGE, 10000, 10000 * MAX_PATH);

        hFont = CreateFont(
        20,                        // Font height (approx. point size, adjust as needed)
        0,                         // Width (0 for default)
        0,                         // Escapement
        0,                         // Orientation
        FW_NORMAL,                 // Weight (FW_NORMAL, FW_BOLD, etc.)
        FALSE,                     // Italic
        FALSE,                     // Underline
        FALSE,                     // Strikeout
        DEFAULT_CHARSET,           // Charset
        OUT_DEFAULT_PRECIS,        // Output precision
        CLIP_DEFAULT_PRECIS,       // Clipping precision
        DEFAULT_QUALITY,           // Quality
        DEFAULT_PITCH | FF_SWISS,  // Pitch and family
        L"Segoe UI"                   // Font family
      );
      SendMessage(list, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    //  labelDir = CreateWindow(L"STATIC", L"Directory:", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 15, 80, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    //   SendMessage(labelDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    //   // Create directory input
    //   editDir = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 90, 10, 420, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    //   SendMessage(editDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    labelDir = CreateWindow(L"STATIC", L"Directory:", WS_CHILD | WS_VISIBLE | SS_LEFT, inputX, 15, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      // Create directory input
      editDir = CreateWindow(L"EDIT", L"C:\\Users\\Douglas\\Code\\win-finder", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, inputX + labelWidth, 10, inputWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(editDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      // Create query label
      labelQuery = CreateWindow(L"STATIC", L"Query:", WS_CHILD | WS_VISIBLE | SS_LEFT, inputX, 45, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelQuery, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

   // Create query input
      editQuery = CreateWindow(L"EDIT", L".c", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, inputX + labelWidth, 40, inputWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(editQuery, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      btnSearch = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, buttonX, 75, buttonWidth, buttonHeight, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
      SendMessage(btnSearch, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      _snwprintf(buffer_scanned, 64, buffer_scanned_format, (s64)0);

      labelScanned = CreateWindow(L"STATIC", buffer_scanned, WS_CHILD | WS_VISIBLE | SS_LEFT, counterX, 15, counterWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelScanned, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      // Create found files label
      _snwprintf(buffer_found, 64, buffer_found_format, (s64)0);

      labelFound = CreateWindow(L"STATIC", buffer_found, WS_CHILD | WS_VISIBLE | SS_LEFT, counterX, 45, counterWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelFound, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      repaint_window(hwnd);
        break;
      }
case LB_ADDSTRING:
    {
      if (l_param)
      {
        LRESULT result = SendMessage(list, LB_ADDSTRING, w_param, l_param);
        free((wchar*)l_param);
        repaint_window(hwnd);
        return result;
      }
      break;
    }

    case WM_TIMER:
    {
      if (w_param == 1)
      {
        // wchar buffer[64];
        _snwprintf(buffer_scanned, 64, buffer_scanned_format, total_files_scanned);
        SetWindowTextW(labelScanned, buffer_scanned);

        _snwprintf(buffer_found, 64, buffer_found_format, total_files_found);
        SetWindowTextW(labelFound, buffer_found);

        if (read_index == write_index) // Stop timer when search completes
          KillTimer(hwnd, 1);
        repaint_window(hwnd);
      }
      break;
    }

      case WM_CTLCOLORSTATIC:
      {
        // Set background color for static controls (labels)
        HDC hdcStatic = (HDC)w_param;
        SetBkColor(hdcStatic, RGB(255, 255, 255)); // White background
        return (LRESULT)hBrush;
      }
      case WM_PAINT:
      {

          //     wchar buffer[32];
          // wsprintfW(buffer, L"Scanned: %lld", total_files_scanned);
          // SetWindowTextW(labelScanned, buffer);
          // wsprintfW(buffer, L"Found: %lld", total_files_found);
          // SetWindowTextW(labelFound, buffer);
        // render_target->BeginDraw();
        // render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));

        // RECT rc;
        // GetClientRect(hwnd, &rc);

        // // wchar buffer[MAX_PATH];
        // // DWORD length = GetCurrentDirectory(MAX_PATH, buffer);
        // // render_target->DrawText(buffer, (u32)length, text_format, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), brush);

        // // wchar *text = L"电视机! Good work! 😎 نطقها نطقًا";
        // wchar *text = results[0];
        // render_target->DrawText(text, (UINT32)wcslen(text), text_format, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), brush);

        // render_target->EndDraw();
        // return 0;
        break;
      }
      case WM_DESTROY:
      {
        PostQuitMessage(0);
        return 0;
      }
      case WM_COMMAND:
      {
        if (LOWORD(w_param) == 1001 && HIWORD(w_param) == BN_CLICKED) // Search button clicked
        {
          // for(s32 i = 0; i < total_threads; i++)
          // {
          //   TerminateThread(threads[i], 0);
          // }

          // WaitForMultipleObjects(total_threads, threads, TRUE, 1000);
          for (s32 i = 0; i < total_threads; i++){
            TerminateThread(threads[i], 0);
            CloseHandle(threads[i]);
          }

            MSG msg__;
        while (PeekMessage(&msg__, list, LB_ADDSTRING, LB_ADDSTRING, PM_REMOVE))
        {
          if (msg__.lParam)
            free((wchar*)msg__.lParam); // Free memory from pending messages
        }

          // Get directory from editDir
          GetWindowText(editDir, dir, MAX_PATH);
          // Get query from editQuery
          GetWindowText(editQuery, query, MAX_PATH);
          // Clear list box
          SendMessage(list, LB_RESETCONTENT, 0, 0);
          // Start search
          list_files_from_directory(dir);
          for(s32 i = 0; i < total_threads; i++)
          {
            threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
          }

          // total_files_scanned = 0; // Reset counters
          // total_files_found = 0;
          // wchar buffer[32];
          // wsprintfW(buffer, L"Scanned: %lld", total_files_scanned);
          // SetWindowTextW(labelScanned, buffer);
          // wsprintfW(buffer, L"Found: %lld", total_files_found);
          // SetWindowTextW(labelFound, buffer);
            // WaitForMultipleObjects(total_threads, threads, true, INFINITE);
            // WaitForMultipleObjects(total_threads, threads, true, 1000);

          // repaint_window(hwnd);

          SetTimer(hwnd, 1, 100, NULL); // Start timer for counter updates
        }
        else if (LOWORD(w_param) == 0 && HIWORD(w_param) == LBN_DBLCLK) // Double-click on list box
      {
        LRESULT index = SendMessage(list, LB_GETCURSEL, 0, 0);
        if (index != LB_ERR)
        {
          wchar text[MAX_PATH];
          SendMessage(list, LB_GETTEXT, index, (LPARAM)text);
          if (OpenClipboard(hwnd))
          {
            EmptyClipboard();
            size_t len = wcslen(text) + 1;
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar));
            if (hMem)
            {
              wchar* clipText = (wchar*)GlobalLock(hMem);
              wcscpy(clipText, text);
              GlobalUnlock(hMem);
              SetClipboardData(CF_UNICODETEXT, hMem);
            }
            CloseClipboard();
          }
        }
      }
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
          case 'A':
          {
            SendMessage(list, LB_ADDSTRING, 0, (LPARAM)L"Item 1");
            repaint_window(hwnd);
            break;
          }
          case VK_SPACE:
          {

            // for(s32 i = 0; i < total_threads; i++)
            // {
            //   threads[i] = CreateThread(null, 0, thread_proc, null, 0, null);
            // }

            // WaitForMultipleObjects(total_threads, threads, true, INFINITE);
            // WaitForMultipleObjects(total_threads, threads, true, 5000);

            // repaint_window(hwnd);
            break;
          }
        }
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
  window_width  = 1280;
  window_height = 720;
  s32 window_x = (screen_width  / 2) - (window_width  / 2);
  s32 window_y = (screen_height / 2) - (window_height / 2);

  window = CreateWindow(
    class_name, L"Win Finder",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    window_x, window_y, window_width, window_height,
    0, 0, 0, 0
  );

  SendMessage(list, LB_INITSTORAGE, 10000, 10000 * MAX_PATH);

  /// D2D Init ///
  
  // ID2D1Factory *pD2DFactory = null;
  // D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

  // IDWriteFactory *pDWriteFactory = null;
  // DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&pDWriteFactory);

  // // ID2D1HwndRenderTarget *render_target = null;
  // RECT rc;
  // GetClientRect(window, &rc);
  // pD2DFactory->CreateHwndRenderTarget
  // (
  //   D2D1::RenderTargetProperties(),
  //   D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right, rc.bottom)),
  //   &render_target
  // );

  // // IDWriteTextFormat *text_format = null;
  // pDWriteFactory->CreateTextFormat(L"", null, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f, L"", &text_format);

  // text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  // text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  // // ID2D1SolidColorBrush *brush = null;
  // render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);
}