// PiGE_LAB3.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PiGE_LAB3.h"
#include <string>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    CustomDisplayWndProc(HWND, UINT, WPARAM, LPARAM);

HWND hDisplay = NULL;
HWND hBtns[18];
HFONT g_hFont = NULL;

HFONT g_hFontHistory = NULL;
HFONT g_hFontCurrent = NULL;

WCHAR szHistory[256] = L"";
WCHAR szCurrent[256] = L"0";

std::wstring currentInput = L"0";
std::wstring historyString = L"";
double previousValue = 0;
int currentOp = 0;
bool newNumber = true;

std::wstring FormatDouble(double d) {
    wchar_t buf[64];
    swprintf_s(buf, L"%g", d);
    return std::wstring(buf);
}

void HandleButtonCommand(int id) {
    if (id >= IDC_BTN_0 && id <= IDC_BTN_9) {
        int d = id - IDC_BTN_0;
        if (newNumber) {
            currentInput = std::to_wstring(d);
            newNumber = false;
        }
        else {
            if (currentInput == L"0") currentInput = std::to_wstring(d);
            else currentInput += std::to_wstring(d);
        }
    }
    else if (id == IDC_BTN_DOT) {
        if (newNumber) { currentInput = L"0."; newNumber = false; }
        else if (currentInput.find(L'.') == std::wstring::npos) currentInput += L".";
    }
    else if (id == IDC_BTN_BS) {
        if (!newNumber && currentInput.length() > 0) {
            currentInput.pop_back();
            if (currentInput.empty() || currentInput == L"-") currentInput = L"0";
        }
    }
    else if (id == IDC_BTN_C || id == IDM_CLEAR) {
        currentInput = L"0";
        historyString = L"";
        previousValue = 0;
        currentOp = 0;
        newNumber = true;
    }
    else if (id >= IDC_BTN_ADD && id <= IDC_BTN_DIV) {
        if (!newNumber || currentOp == 0) {
            if (currentOp != 0) {
                double currentValue = std::stod(currentInput);
                if (currentOp == IDC_BTN_ADD) previousValue += currentValue;
                if (currentOp == IDC_BTN_SUB) previousValue -= currentValue;
                if (currentOp == IDC_BTN_MUL) previousValue *= currentValue;
                if (currentOp == IDC_BTN_DIV && currentValue != 0) previousValue /= currentValue;
            }
            else {
                previousValue = std::stod(currentInput);
            }
        }
        currentOp = id;
        newNumber = true;
        wchar_t opChar = (id == IDC_BTN_ADD) ? L'+' : (id == IDC_BTN_SUB) ? L'-' : (id == IDC_BTN_MUL) ? L'*' : L'/';
        historyString = FormatDouble(previousValue) + L" " + opChar;
        currentInput = FormatDouble(previousValue);
    }
    else if (id == IDC_BTN_EQ) {
        if (currentOp != 0) {
            double currentValue = std::stod(currentInput);
            if (currentOp == IDC_BTN_ADD) previousValue += currentValue;
            if (currentOp == IDC_BTN_SUB) previousValue -= currentValue;
            if (currentOp == IDC_BTN_MUL) previousValue *= currentValue;
            if (currentOp == IDC_BTN_DIV && currentValue != 0) previousValue /= currentValue;
            currentInput = FormatDouble(previousValue);
            historyString = L"";
            currentOp = 0;
            newNumber = true;
        }
    }
    wcscpy_s(szCurrent, currentInput.c_str());
    wcscpy_s(szHistory, historyString.c_str());
}

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

    RegisterClassExW(&wcex);

    WNDCLASSEXW wcexDisp = { 0 };
    wcexDisp.cbSize = sizeof(WNDCLASSEX);
    wcexDisp.style = CS_HREDRAW | CS_VREDRAW;
    wcexDisp.lpfnWndProc = CustomDisplayWndProc;
    wcexDisp.hInstance = hInstance;
    wcexDisp.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcexDisp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcexDisp.lpszClassName = L"CustomDisplay";

    return RegisterClassExW(&wcexDisp);

}


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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        HMENU hMenu = GetMenu(hWnd);
        CheckMenuItem(hMenu, IDM_BASIC, MF_CHECKED);

        hDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"CustomDisplay", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
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

        int displayHeight = (cy / 4);

        HDWP hdwp = BeginDeferWindowPos(19);

        hdwp = DeferWindowPos(hdwp, hDisplay, NULL, gap, gap, cx - (2 * gap), displayHeight - gap, SWP_NOZORDER);

        int startY = displayHeight + gap;
        int btnAreaHeight = cy - startY;

        int btnWidth = (cx - (5 * gap)) / 4;
        int btnHeight = (btnAreaHeight - (6 * gap)) / 5;

        for (int i = 0; i < 16; i++) {
            int row = i / 4;
            int col = i % 4;
            int x = gap + col * (btnWidth + gap);
            int y = startY + row * (btnHeight + gap);
            hdwp = DeferWindowPos(hdwp, hBtns[i], NULL, x, y, btnWidth, btnHeight, SWP_NOZORDER);
        }

        // BS
        int x = gap;
        int y = startY + 4 * (btnHeight + gap);
        hdwp = DeferWindowPos(hdwp, hBtns[16], NULL, x, y, btnWidth, btnHeight, SWP_NOZORDER);


        // =
        x = gap + btnWidth + gap;
        int eqWidth = cx - gap - x;
        hdwp = DeferWindowPos(hdwp, hBtns[17], NULL, x, y, eqWidth, btnHeight, SWP_NOZORDER); 

        EndDeferWindowPos(hdwp);

        if (g_hFont) DeleteObject(g_hFont);
        if (g_hFontCurrent) DeleteObject(g_hFontCurrent);
        if (g_hFontHistory) DeleteObject(g_hFontHistory);

        int fontSize = btnHeight * 4 / 10;
        if (fontSize < 16) fontSize = 16;

        g_hFont = CreateFontW(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontCurrent = CreateFontW(displayHeight * 2 / 3, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontHistory = CreateFontW(displayHeight / 4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        for (int i = 0; i < 18; i++) {
            SendMessage(hBtns[i], WM_SETFONT, (WPARAM)g_hFont, TRUE);
        }

        InvalidateRect(hDisplay, NULL, TRUE); // Repaint
        return 0;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            if (wmId >= IDC_BTN_0 && wmId <= IDC_BTN_DOT) {
                HandleButtonCommand(wmId);
                InvalidateRect(hDisplay, NULL, TRUE);
                SetFocus(hWnd);
                return 0;
            }

            switch (wmId)
            {
            case IDM_PROGRAMMER:
                MessageBoxW(hWnd, L"Not implemented yet", L"Programmer Mode", MB_OK | MB_ICONINFORMATION);
                break;
            case IDM_CLEAR:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_BACK) PostMessage(hWnd, WM_COMMAND, IDC_BTN_BS, 0);
        else if (wParam == VK_RETURN) PostMessage(hWnd, WM_COMMAND, IDC_BTN_EQ, 0);
        break;

    case WM_CHAR:
        if (wParam >= '0' && wParam <= '9') PostMessage(hWnd, WM_COMMAND, IDC_BTN_0 + (wParam - '0'), 0);
        else if (wParam == '+') PostMessage(hWnd, WM_COMMAND, IDC_BTN_ADD, 0);
        else if (wParam == '-') PostMessage(hWnd, WM_COMMAND, IDC_BTN_SUB, 0);
        else if (wParam == '*') PostMessage(hWnd, WM_COMMAND, IDC_BTN_MUL, 0);
        else if (wParam == '/') PostMessage(hWnd, WM_COMMAND, IDC_BTN_DIV, 0);
        else if (wParam == '.' || wParam == ',') PostMessage(hWnd, WM_COMMAND, IDC_BTN_DOT, 0);
        else if (wParam == '=') PostMessage(hWnd, WM_COMMAND, IDC_BTN_EQ, 0);
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
        if (g_hFont) DeleteObject(g_hFont);
        if (g_hFontCurrent) DeleteObject(g_hFontCurrent);
        if (g_hFontHistory) DeleteObject(g_hFontHistory);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK CustomDisplayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        RECT rcHistory = rc;
        rcHistory.bottom = rc.bottom / 3;
        rcHistory.right -= 5;

        RECT rcCurrent = rc;
        rcCurrent.top = rc.bottom / 3;
        rcCurrent.right -= 5;

        SetBkMode(hdc, TRANSPARENT);

        if (g_hFontHistory) SelectObject(hdc, g_hFontHistory);
        SetTextColor(hdc, RGB(100, 100, 100));
        DrawTextW(hdc, szHistory, -1, &rcHistory, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

        if (g_hFontCurrent) SelectObject(hdc, g_hFontCurrent);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawTextW(hdc, szCurrent, -1, &rcCurrent, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
        return 1;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}
