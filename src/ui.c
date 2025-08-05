s32 window_width  = 600;
s32 window_height = 600;

wchar buffer_scanned[64] = {0};
#define buffer_scanned_format L"Total files scanned: %lld"

wchar buffer_found[64] = {0};
#define buffer_found_format L"Total files found: %lld"

void repaint_window(HWND hwnd)
{
  InvalidateRect(hwnd, null, true);
  UpdateWindow(hwnd);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
  static HWND editDir;
  static HWND editQuery;
  static HWND btnSearch;
  static HWND labelDir;
  static HWND labelQuery;
  static HBRUSH hBrush;
  static HFONT hFont;

  switch(msg)
  {
    case WM_CREATE:
    {
      hBrush = CreateSolidBrush(RGB(255, 255, 255));

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
      SendMessage(list, LB_INITSTORAGE, 10000, 10000 * MAX_PATH);

      hFont = CreateFont(
        20,
        0, 0, 0,
        FW_NORMAL,
        false, false, false,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
        L"Segoe UI"
      );
      SendMessage(list, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      labelDir = CreateWindow(L"STATIC", L"Directory:", WS_CHILD | WS_VISIBLE | SS_LEFT, inputX, 15, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      // Create directory input
      editDir = CreateWindow(L"EDIT", L"C:\\Users\\Douglas\\Code\\win-finder", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, inputX + labelWidth, 10, inputWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(editDir, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      // Create query label
      labelQuery = CreateWindow(L"STATIC", L"Query:", WS_CHILD | WS_VISIBLE | SS_LEFT, inputX, 45, labelWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(labelQuery, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      editQuery = CreateWindow(L"EDIT", L".c", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, inputX + labelWidth, 40, inputWidth, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(editQuery, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      btnSearch = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, buttonX, 75, buttonWidth, buttonHeight, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
      SendMessage(btnSearch, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      _snwprintf(buffer_scanned, 64, buffer_scanned_format, (s64)0);

      label_scanned = CreateWindow(L"STATIC", buffer_scanned, WS_CHILD | WS_VISIBLE | SS_LEFT, counterX, 15, counterWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(label_scanned, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      _snwprintf(buffer_found, 64, buffer_found_format, (s64)0);

      label_found = CreateWindow(L"STATIC", buffer_found, WS_CHILD | WS_VISIBLE | SS_LEFT, counterX, 45, counterWidth, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
      SendMessage(label_found, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

      repaint_window(hwnd);
      break;
    }
    case WM_TIMER:
    {
      if(w_param == 1)
      {
        _snwprintf(buffer_scanned, 64, buffer_scanned_format, total_files_scanned);
        SetWindowTextW(label_scanned, buffer_scanned);

        _snwprintf(buffer_found, 64, buffer_found_format, total_files_found);
        SetWindowTextW(label_found, buffer_found);

        if(read_index == write_index)
        {
          KillTimer(hwnd, 1);
        }

        repaint_window(hwnd);
      }
      break;
    }
    case WM_CTLCOLORSTATIC: // Needed for painting the labels background white.
    {
      HDC hdcStatic = (HDC)w_param;
      SetBkColor(hdcStatic, RGB(255, 255, 255));
      return (LRESULT)hBrush;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    case WM_COMMAND:
    {
      // Search button clicked
      if(LOWORD(w_param) == 1001 && HIWORD(w_param) == BN_CLICKED)
      {
        terminate_threads();

        MSG msg__;
        while (PeekMessage(&msg__, list, LB_ADDSTRING, LB_ADDSTRING, PM_REMOVE))
        {
          if(msg__.lParam)
            free((wchar*)msg__.lParam); // Free memory from pending messages
        }

        total_files_scanned = 0;
        total_files_found = 0;

        GetWindowText(editDir, dir, MAX_PATH);
        GetWindowText(editQuery, query, MAX_PATH);

        SendMessage(list, LB_RESETCONTENT, 0, 0);

        list_files_from_directory(dir);
        run_threads();

        SetTimer(hwnd, 1, 300, null); // Start timer for counter updates
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
            if(hMem)
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
}