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

// Programmer

enum AppMode { MODE_BASIC, MODE_PROGRAMMER };
AppMode currentMode = MODE_BASIC;

HWND hHexBtns[6];
HWND hComboType;
HWND hRadioBases[4];

WCHAR szIniPath[MAX_PATH];

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
    else if (id == IDC_BTN_CLEAR || id == IDM_CLEAR) {
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

   GetCurrentDirectoryW(MAX_PATH, szIniPath);
   wcscat_s(szIniPath, MAX_PATH, L"\\config.ini");

   int x = GetPrivateProfileIntW(L"Window", L"X", 600, szIniPath);
   int y = GetPrivateProfileIntW(L"Window", L"Y", 100, szIniPath);
   int cx = GetPrivateProfileIntW(L"Window", L"Width", 350, szIniPath);
   int cy = GetPrivateProfileIntW(L"Window", L"Height", 450, szIniPath);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       x, y, cx, cy, nullptr, nullptr, hInstance, nullptr);

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
        currentMode = (AppMode)GetPrivateProfileIntW(L"Settings", L"Mode", MODE_BASIC, szIniPath);

        if (currentMode == MODE_PROGRAMMER) {
            CheckMenuItem(hMenu, IDM_PROGRAMMER, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_BASIC, MF_UNCHECKED);
        }
        else {
            CheckMenuItem(hMenu, IDM_BASIC, MF_CHECKED);
            CheckMenuItem(hMenu, IDM_PROGRAMMER, MF_UNCHECKED);
        }

        hDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"CustomDisplay", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            0, 0, 0, 0, hWnd, (HMENU)IDC_DISPLAY, GetModuleHandle(NULL), NULL);

        struct ButtonDef { int id; const wchar_t* label; };
        ButtonDef layout[18] = {
            { IDC_BTN_7, L"7" }, { IDC_BTN_8, L"8" }, { IDC_BTN_9, L"9" }, { IDC_BTN_DIV, L"/" },
            { IDC_BTN_4, L"4" }, { IDC_BTN_5, L"5" }, { IDC_BTN_6, L"6" }, { IDC_BTN_MUL, L"*" },
            { IDC_BTN_1, L"1" }, { IDC_BTN_2, L"2" }, { IDC_BTN_3, L"3" }, { IDC_BTN_SUB, L"-" },
            {IDC_BTN_CLEAR, L"CL"}, { IDC_BTN_0, L"0" }, { IDC_BTN_DOT, L"." }, { IDC_BTN_ADD, L"+" },
            { IDC_BTN_BS, L"BS" }, { IDC_BTN_EQ, L"=" }
        };

        for (int i = 0; i < 18; i++) {
            hBtns[i] = CreateWindowExW(0, L"Button", layout[i].label,
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hWnd, (HMENU)(UINT_PTR)layout[i].id, GetModuleHandle(NULL), NULL);
        }

        ButtonDef hexLayout[6] = {
            {IDC_BTN_A, L"A"}, {IDC_BTN_B, L"B"}, {IDC_BTN_C, L"C"},
            {IDC_BTN_D, L"D"}, {IDC_BTN_E, L"E"}, {IDC_BTN_F, L"F"}
        };
        for (int i = 0; i < 6; i++) {
            hHexBtns[i] = CreateWindowExW(0, L"Button", hexLayout[i].label,
                WS_CHILD | BS_PUSHBUTTON, // No WS_VISIBLE
                0, 0, 0, 0, hWnd, (HMENU)(UINT_PTR)hexLayout[i].id, GetModuleHandle(NULL), NULL);
        }

        hComboType = CreateWindowExW(0, L"COMBOBOX", L"",
            WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
            0, 0, 0, 0, hWnd, (HMENU)IDC_COMBO_TYPE, GetModuleHandle(NULL), NULL);

        const wchar_t* dataTypes[] = { L"64-bit QWORD", L"32-bit DWORD", L"16-bit WORD", L"8-bit BYTE", L"Float (32-bit)", L"Double (64-bit)" };
        for (const wchar_t* type : dataTypes) {
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)type);
        }
        SendMessage(hComboType, CB_SETCURSEL, 0, 0);

        ButtonDef radioLayout[4] = {
            {IDC_RADIO_HEX, L"HEX"}, {IDC_RADIO_DEC, L"DEC"}, {IDC_RADIO_OCT, L"OCT"}, {IDC_RADIO_BIN, L"BIN"}
        };
        for (int i = 0; i < 4; i++) {
            hRadioBases[i] = CreateWindowExW(0, L"Button", radioLayout[i].label,
                WS_CHILD | BS_AUTORADIOBUTTON,
                0, 0, 0, 0, hWnd, (HMENU)(UINT_PTR)radioLayout[i].id, GetModuleHandle(NULL), NULL);
        }

        int savedDataType = GetPrivateProfileIntW(L"Settings", L"DataType", 0, szIniPath);
        SendMessage(hComboType, CB_SETCURSEL, savedDataType, 0);

        int savedBase = GetPrivateProfileIntW(L"Settings", L"Base", 1, szIniPath);
        SendMessage(hRadioBases[savedBase], BM_SETCHECK, BST_CHECKED, 0);

        return 0;
        }

    case WM_SIZE: {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        int gap = 5;

        int displayHeight = (cy / 4);

        int showCmd = (currentMode == MODE_PROGRAMMER) ? SW_SHOW : SW_HIDE;
        ShowWindow(hComboType, showCmd);
        for (int i = 0; i < 6; i++) ShowWindow(hHexBtns[i], showCmd);
        for (int i = 0; i < 4; i++) ShowWindow(hRadioBases[i], showCmd);

        HDWP hdwp = BeginDeferWindowPos(30);


        if (currentMode == MODE_BASIC) {

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

            if (g_hFont) DeleteObject(g_hFont);
            int fontSize = max(16, btnHeight * 4 / 10);
            g_hFont = CreateFontW(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            for (int i = 0; i < 18; i++) SendMessage(hBtns[i], WM_SETFONT, (WPARAM)g_hFont, TRUE);
        }
        else {

            displayHeight = (cy / 6);

            hdwp = DeferWindowPos(hdwp, hDisplay, NULL, gap, gap, cx - (2 * gap), displayHeight - gap, SWP_NOZORDER);

            int bitDisplayHeight = displayHeight * 3 / 4;
            // DeferWindowPos for bitDisplay here
            // x: gap, y: displayHeight + gap, height: bitDisplayHeight

            int comboHeight = 25;
            int startY = displayHeight + gap + bitDisplayHeight + gap;

            hdwp = DeferWindowPos(hdwp, hComboType, NULL, gap, startY, cx - (2 * gap), comboHeight * 5, SWP_NOZORDER);
            startY += comboHeight + gap;

            for (int i = 0; i < 4; i++) {
                ShowWindow(hRadioBases[i], SW_HIDE);
            }

            int btnAreaHeight = cy - startY;
            int btnWidth = (cx - (5 * gap)) / 4;
            int btnHeight = (btnAreaHeight - (8 * gap)) / 7;

            // hBtns mapping: 0=7, 1=8, 2=9, 3=/, 4=4, 5=5, 6=6, 7=*, 8=1, 9=2, 10=3, 11=-, 12=C(Clear), 13=0, 14=., 15=+, 16=BS, 17==
            // hHexBtns mapping: 0=A, 1=B, 2=C, 3=D, 4=E, 5=F
            HWND progGrid[7][4] = {
                { hHexBtns[0], hHexBtns[1], hHexBtns[2], hHexBtns[3] }, // A, B, C, D
                { hHexBtns[4], hHexBtns[5], NULL,        NULL        }, // E, F, (empty), (empty)
                { hBtns[0],    hBtns[1],    hBtns[2],    hBtns[3]    }, // 7, 8, 9, /
                { hBtns[4],    hBtns[5],    hBtns[6],    hBtns[7]    }, // 4, 5, 6, *
                { hBtns[8],    hBtns[9],    hBtns[10],   hBtns[11]   }, // 1, 2, 3, -
                { hBtns[12],   hBtns[13],   hBtns[14],   hBtns[15]   }, // C, 0, ., +
                { hBtns[16],   hBtns[17],   NULL,        NULL        }  // BS, =
            };

            for (int row = 0; row < 7; row++) {
                for (int col = 0; col < 4; col++) {
                    HWND hItem = progGrid[row][col];
                    if (!hItem) continue;

                    int x = gap + col * (btnWidth + gap);
                    int y = startY + row * (btnHeight + gap);
                    int w = btnWidth;

                    if (hItem == hBtns[17]) {
                        w = (btnWidth * 3) + (gap * 2);
                    }

                    hdwp = DeferWindowPos(hdwp, hItem, NULL, x, y, w, btnHeight, SWP_NOZORDER);
                }
            }

            if (g_hFont) DeleteObject(g_hFont);
            int fontSize = max(14, btnHeight * 3 / 10);
            g_hFont = CreateFontW(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

            for (int i = 0; i < 18; i++) SendMessage(hBtns[i], WM_SETFONT, (WPARAM)g_hFont, TRUE);
            for (int i = 0; i < 6; i++) SendMessage(hHexBtns[i], WM_SETFONT, (WPARAM)g_hFont, TRUE);
        }

        EndDeferWindowPos(hdwp);

        if (g_hFontCurrent) DeleteObject(g_hFontCurrent);
        if (g_hFontHistory) DeleteObject(g_hFontHistory);
        g_hFontCurrent = CreateFontW(displayHeight * 2 / 3, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontHistory = CreateFontW(displayHeight / 4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        InvalidateRect(hWnd, NULL, TRUE);
        InvalidateRect(hDisplay, NULL, TRUE);

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

            if (wmId >= IDC_BTN_A && wmId <= IDC_BTN_F) {
                // HandleHexInput(wmId);
                SetFocus(hWnd);
                return 0;
            }

            RECT rc;

            switch (wmId)
            {
            case IDM_BASIC:
                currentMode = MODE_BASIC;
                CheckMenuItem(GetMenu(hWnd), IDM_BASIC, MF_CHECKED);
                CheckMenuItem(GetMenu(hWnd), IDM_PROGRAMMER, MF_UNCHECKED);

                GetClientRect(hWnd, &rc);
                SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));

                break;
            case IDM_PROGRAMMER:
                currentMode = MODE_PROGRAMMER;
                CheckMenuItem(GetMenu(hWnd), IDM_PROGRAMMER, MF_CHECKED);
                CheckMenuItem(GetMenu(hWnd), IDM_BASIC, MF_UNCHECKED);

                GetClientRect(hWnd, &rc);
                SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));
                break;
            case IDM_BASE_HEX:
                SendMessage(hRadioBases[0], BM_CLICK, 0, 0); break;
            case IDM_BASE_DEC:
                SendMessage(hRadioBases[1], BM_CLICK, 0, 0); break;
            case IDM_BASE_OCT:
                SendMessage(hRadioBases[2], BM_CLICK, 0, 0); break;
            case IDM_BASE_BIN: 
                SendMessage(hRadioBases[3], BM_CLICK, 0, 0); break;
            case IDM_CLEAR:
                HandleButtonCommand(IDC_BTN_CLEAR);
                InvalidateRect(hDisplay, NULL, TRUE);
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
        else if (currentMode == MODE_PROGRAMMER) {
            if (wParam >= 'A' && wParam <= 'F') PostMessage(hWnd, WM_COMMAND, IDC_BTN_A + (wParam - 'A'), 0);
            else if (wParam >= 'a' && wParam <= 'f') PostMessage(hWnd, WM_COMMAND, IDC_BTN_A + (wParam - 'a'), 0);
        }
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
    {
        // Save to config.ini

        RECT rc;
        GetWindowRect(hWnd, &rc);
        WritePrivateProfileStringW(L"Window", L"X", std::to_wstring(rc.left).c_str(), szIniPath);
        WritePrivateProfileStringW(L"Window", L"Y", std::to_wstring(rc.top).c_str(), szIniPath);
        WritePrivateProfileStringW(L"Window", L"Width", std::to_wstring(rc.right - rc.left).c_str(), szIniPath);
        WritePrivateProfileStringW(L"Window", L"Height", std::to_wstring(rc.bottom - rc.top).c_str(), szIniPath);

        WritePrivateProfileStringW(L"Settings", L"Mode", std::to_wstring(currentMode).c_str(), szIniPath);

        int selectedType = (int)SendMessage(hComboType, CB_GETCURSEL, 0, 0);
        WritePrivateProfileStringW(L"Settings", L"DataType", std::to_wstring(selectedType).c_str(), szIniPath);

        int selectedBase = 1; // Decimal
        for (int i = 0; i < 4; i++) {
            if (SendMessage(hRadioBases[i], BM_GETCHECK, 0, 0) == BST_CHECKED) {
                selectedBase = i;
                break;
            }
        }
        WritePrivateProfileStringW(L"Settings", L"Base", std::to_wstring(selectedBase).c_str(), szIniPath);

        if (g_hFont) DeleteObject(g_hFont);
        if (g_hFontCurrent) DeleteObject(g_hFontCurrent);
        if (g_hFontHistory) DeleteObject(g_hFontHistory);
        PostQuitMessage(0);
        break;
    }
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
