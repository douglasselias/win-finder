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

void set_font(HFONT font, HWND hwnd)
{
  SendMessage(hwnd, WM_SETFONT, (WPARAM)font, MAKELPARAM(true, 0));
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
  static HWND label_dir;
  static HWND input_dir;
  static HWND label_query;
  static HWND input_query;
  static HWND button_search;
  static HWND checkbox_fuzzy;

  static u64 button_search_id  = 1001;
  static u64 checkbox_fuzzy_id = 1002;
  static u64 listbox_id        = 1003;

  static HBRUSH brush;
  static HFONT font;

  switch(msg)
  {
    case WM_CREATE:
    {
      brush = CreateSolidBrush(RGB(255, 255, 255));

      font = CreateFont(
        20,
        0, 0, 0,
        FW_NORMAL,
        false, false, false,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
        L"Segoe UI"
      );

      RECT client_rect;
      GetClientRect(hwnd, &client_rect);
      s32 client_width = client_rect.right - client_rect.left;

      s32 label_width = 80;
      s32 input_width = 420;
      s32 button_width = 90;
      s32 button_height = 30;
      s32 counter_width = 200;
      s32 checkbox_width = 150;

      s32 total_input_width = label_width + input_width;

      s32 input_x = (client_width - total_input_width) / 2;
      s32 button_x = (client_width - button_width) / 2;
      s32 counter_x = input_x + total_input_width + 10;
      s32 checkbox_x = (client_width - checkbox_width) / 2;

      list = CreateWindow(L"LISTBOX", null,  WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | WS_BORDER, 10, 150, window_width - (10 * 4), window_height - (10 * 16), hwnd, (HMENU)listbox_id, GetModuleHandle(null), null);
      set_font(font, list);

      label_dir = CreateWindow(L"STATIC", L"Directory:", WS_CHILD | WS_VISIBLE | SS_LEFT, input_x, 15, label_width, 20, hwnd, null, GetModuleHandle(null), null);
      set_font(font, label_dir);

      input_dir = CreateWindow(L"EDIT", L"C:\\Users\\Douglas\\Code\\tmp\\win_finder_test_folder", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, input_x + label_width, 10, input_width, 25, hwnd, null, GetModuleHandle(null), null);
      set_font(font, input_dir);

      label_query = CreateWindow(L"STATIC", L"Query:", WS_CHILD | WS_VISIBLE | SS_LEFT, input_x, 45, label_width, 20, hwnd, null, GetModuleHandle(null), null);
      set_font(font, label_query);

      input_query = CreateWindow(L"EDIT", L"main.cpp", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, input_x + label_width, 40, input_width, 25, hwnd, null, GetModuleHandle(null), null);
      set_font(font, input_query);

      button_search = CreateWindow(L"BUTTON", L"Search", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, button_x, 110, button_width, button_height, hwnd, (HMENU)button_search_id, GetModuleHandle(null), null);
      set_font(font, button_search);

      _snwprintf(buffer_scanned, 64, buffer_scanned_format, (s64)0);

      label_scanned = CreateWindow(L"STATIC", buffer_scanned, WS_CHILD | WS_VISIBLE | SS_LEFT, counter_x, 15, counter_width, 20, hwnd, null, GetModuleHandle(null), null);
      set_font(font, label_scanned);

      _snwprintf(buffer_found, 64, buffer_found_format, (s64)0);

      label_found = CreateWindow(L"STATIC", buffer_found, WS_CHILD | WS_VISIBLE | SS_LEFT, counter_x, 45, counter_width, 20, hwnd, null, GetModuleHandle(null), null);
      set_font(font, label_found);

      checkbox_fuzzy = CreateWindow(L"BUTTON", L"Enable fuzzy search", WS_CHILD | WS_VISIBLE | BS_CHECKBOX, checkbox_x, 75, checkbox_width, 20, hwnd, (HMENU)1002, GetModuleHandle(null), null);
      set_font(font, checkbox_fuzzy);

      break;
    }
    case WM_TIMER:
    {
      _snwprintf(buffer_scanned, 64, buffer_scanned_format, total_files_scanned);
      SetWindowText(label_scanned, buffer_scanned);

      _snwprintf(buffer_found, 64, buffer_found_format, total_files_found);
      SetWindowText(label_found, buffer_found);

      if(read_index == write_index)
      {
        KillTimer(hwnd, 1);
      }

      repaint_window(hwnd);

      break;
    }
    case WM_CTLCOLORSTATIC: // Needed for painting the labels background white.
    {
      SetBkColor((HDC)w_param, RGB(255, 255, 255));
      return (LRESULT)brush;
    }
    case WM_DESTROY:
    {
      PostQuitMessage(0);
      return 0;
    }
    case WM_COMMAND:
    {
      // Search button clicked
      if(LOWORD(w_param) == button_search_id && HIWORD(w_param) == BN_CLICKED)
      {
        total_files_scanned = 0;
        total_files_found   = 0;

        GetWindowText(input_dir,   dir,   MAX_PATH);
        GetWindowText(input_query, query, MAX_PATH);

        SendMessage(list, LB_RESETCONTENT, 0, 0);

        list_files_from_directory(dir);
        run_threads();

        SetTimer(hwnd, 1, 300, null); // Start timer for counter updates
      }
      else if(LOWORD(w_param) == checkbox_fuzzy_id && HIWORD(w_param) == BN_CLICKED)
      {
        LRESULT checked = SendMessage(checkbox_fuzzy, BM_GETCHECK, 0, 0);
        if(checked == BST_CHECKED)
        {
          SendMessage(checkbox_fuzzy, BM_SETCHECK, BST_UNCHECKED, 0);
          string_match_proc = has_substring;
        }
        else
        {
          SendMessage(checkbox_fuzzy, BM_SETCHECK, BST_CHECKED, 0);
          string_match_proc = simple_fuzzy_match;
        }
      }
      // Double click to copy item list path to clipboard
      else if(LOWORD(w_param) == listbox_id && HIWORD(w_param) == LBN_DBLCLK)
      {
        LRESULT index = SendMessage(list, LB_GETCURSEL, 0, 0);
        wchar path[MAX_PATH];
        SendMessage(list, LB_GETTEXT, index, (LPARAM)path);

        OpenClipboard(hwnd);
        EmptyClipboard();

        size_t len = wcslen(path) + 1;
        HGLOBAL clip_text_memory = GlobalAlloc(GMEM_MOVEABLE, len * sizeof(wchar));

        wchar* clip_text = (wchar*)GlobalLock(clip_text_memory);
        wcscpy(clip_text, path);
        GlobalUnlock(clip_text_memory);

        SetClipboardData(CF_UNICODETEXT, clip_text_memory);
        CloseClipboard();
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