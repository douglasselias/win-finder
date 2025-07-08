ID2D1HwndRenderTarget *pRenderTarget = null;
IDWriteTextFormat *pTextFormat = null;
ID2D1SolidColorBrush *pBlackBrush = null;

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    switch(msg)
    {
      case WM_PAINT:
      {
        pRenderTarget->BeginDraw();
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        RECT rc;
        GetClientRect(hwnd, &rc);

        // wchar buffer[MAX_PATH];
        // DWORD length = GetCurrentDirectory(MAX_PATH, buffer);
        // pRenderTarget->DrawText(buffer, (u32)length, pTextFormat, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), pBlackBrush);

        wchar *text = L"电视机! Good work! 😎 نطقها نطقًا";
        pRenderTarget->DrawText(text, (UINT32)wcslen(text), pTextFormat, D2D1::RectF(0, 0, (f32)rc.right, (f32)rc.bottom), pBlackBrush);

        pRenderTarget->EndDraw();
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

  /// D2D Init ///
  
  ID2D1Factory *pD2DFactory = null;
  D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

  IDWriteFactory *pDWriteFactory = null;
  DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&pDWriteFactory);

  // ID2D1HwndRenderTarget *pRenderTarget = null;
  RECT rc;
  GetClientRect(window, &rc);
  pD2DFactory->CreateHwndRenderTarget
  (
    D2D1::RenderTargetProperties(),
    D2D1::HwndRenderTargetProperties(window, D2D1::SizeU(rc.right, rc.bottom)),
    &pRenderTarget
  );

  // IDWriteTextFormat *pTextFormat = null;
  pDWriteFactory->CreateTextFormat(L"", null, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f, L"", &pTextFormat);

  pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
  pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

  // ID2D1SolidColorBrush *pBlackBrush = null;
  pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBlackBrush);
}