// PiGE_LAB3.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PiGE_LAB3.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hDisplay = NULL;
HWND hBtns[18];
HFONT g_hFont = NULL;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIGELAB3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIGELAB3));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIGELAB3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PIGELAB3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      600, 0, 350, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        hDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY | ES_AUTOHSCROLL,
            0, 0, 0, 0, hWnd, (HMENU)IDC_DISPLAY, GetModuleHandle(NULL), NULL);

        struct ButtonDef { int id; const wchar_t* label; };
        ButtonDef layout[18] = {
            { IDC_BTN_7, L"7" }, { IDC_BTN_8, L"8" }, { IDC_BTN_9, L"9" }, { IDC_BTN_DIV, L"/" },
            { IDC_BTN_4, L"4" }, { IDC_BTN_5, L"5" }, { IDC_BTN_6, L"6" }, { IDC_BTN_MUL, L"*" },
            { IDC_BTN_1, L"1" }, { IDC_BTN_2, L"2" }, { IDC_BTN_3, L"3" }, { IDC_BTN_SUB, L"-" },
            {IDC_BTN_C, L"C"}, { IDC_BTN_0, L"0" }, { IDC_BTN_DOT, L"." }, { IDC_BTN_ADD, L"+" },
            { IDC_BTN_BS, L"BS" }, { IDC_BTN_EQ, L"=" }
        };

        for (int i = 0; i < 18; i++) {
            hBtns[i] = CreateWindowExW(0, L"Button", layout[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hWnd, (HMENU)(UINT_PTR)layout[i].id, GetModuleHandle(NULL), NULL);
        }

        return 0;
        }

    case WM_SIZE: {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);

        int gap = 5;

        int displayHeight = (cy / 6);
        MoveWindow(hDisplay, gap, gap, cx - (2 * gap), displayHeight - gap, TRUE);

        int startY = displayHeight + gap;
        int btnAreaHeight = cy - startY;

        int btnWidth = (cx - (5 * gap)) / 4;
        int btnHeight = (btnAreaHeight - (6 * gap)) / 5;

        for (int i = 0; i < 16; i++) {
            int row = i / 4;
            int col = i % 4;

            int x = gap + col * (btnWidth + gap);
            int y = startY + row * (btnHeight + gap);

            MoveWindow(hBtns[i], x, y, btnWidth, btnHeight, TRUE);
        }
        int x, y;
        x = gap;
        y = startY + 4 * (btnHeight + gap);

        MoveWindow(hBtns[16], x, y, btnWidth, btnHeight, TRUE);

        x = gap + 1 * (btnWidth + gap);
        y = startY + 4 * (btnHeight + gap);

        btnWidth = (cx - 2*gap) - btnWidth;

        MoveWindow(hBtns[17], x, y, btnWidth, btnHeight, TRUE);

        int fontSize = btnHeight * 4 / 10;
        if (fontSize < 16) fontSize = 16;

        g_hFont = CreateFontW(
            fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
        );

        SendMessage(hDisplay, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        for (int i = 0; i < 18; i++) {
            SendMessage(hBtns[i], WM_SETFONT, (WPARAM)g_hFont, TRUE);
        }
        
        return 0;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_PROGRAMMER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_CLEAR:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 320;
        lpMMI->ptMinTrackSize.y = 400;
        return 0;
    }

    case WM_DESTROY:
        if (g_hFont != NULL) {
            DeleteObject(g_hFont);
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
