// PiGE_LAB3.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PiGE_LAB3.h"
#include <string>

#include <sstream>
#include <iomanip>
#include <bitset>

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
LRESULT CALLBACK    CustomBitDisplayWndProc(HWND, UINT, WPARAM, LPARAM);

HWND hDisplay = NULL;
HWND hBtns[18];
HFONT g_hFont = NULL;

HFONT g_hFontHistory = NULL;
HFONT g_hFontCurrent = NULL;
HFONT g_hFontWarning = NULL;

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
bool isAlwaysOnTop = false;

HWND hBitDisplay = NULL;
bool useSeparators = false;

enum DataType {
    DT_INT8, DT_UINT8, DT_INT16, DT_UINT16,
    DT_INT32, DT_UINT32, DT_INT64, DT_UINT64,
    DT_HALF, DT_FLOAT, DT_DOUBLE
};
DataType currentDataType = DT_INT64;
int currentBase = 10; // 16 (Hex), 10 (Dec), 8 (Oct), 2 (Bin)

union CalcMemory {
    int8_t   i8;  uint8_t   u8;
    int16_t  i16; uint16_t  u16; // Half-float is stored in u16
    int32_t  i32; uint32_t  u32;
    int64_t  i64; uint64_t  u64;
    float    f32;
    double   f64;
};
CalcMemory calcMem = { 0 };
bool showPrecisionWarning = false;
std::wstring exactDecimalInput = L"0";

// Float16 conversions based on: https://gist.github.com/zhuker/b4bd1fb306c7b04975b712c37c4c4075

float HalfToFloat(uint16_t h) {
    uint32_t t1 = h & 0x7fffu;             // Non-sign bits
    uint32_t t2 = h & 0x8000u;             // Sign bit
    uint32_t t3 = h & 0x7c00u;             // Exponent

    t1 <<= 13u;                            // Align mantissa on MSB
    t2 <<= 16u;                            // Shift sign bit into position

    t1 += 0x38000000;                      // Adjust bias
    t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero

    t1 |= t2;                              // Re-insert sign bit

    float out;
    *((uint32_t*)&out) = t1;
    return out;
}

uint16_t FloatToHalf(float f) {

    uint32_t inu = *((uint32_t*)&f);

    uint32_t t1 = inu & 0x7fffffffu;       // Non-sign bits
    uint32_t t2 = inu & 0x80000000u;       // Sign bit
    uint32_t t3 = inu & 0x7f800000u;       // Exponent

    t1 >>= 13u;                            // Align mantissa on MSB
    t2 >>= 16u;                            // Shift sign bit into position

    t1 -= 0x1c000;                         // Adjust bias
    t1 = (t3 < 0x38800000u) ? 0 : t1;      // Flush-to-zero
    t1 = (t3 > 0x8e000000u) ? 0x7bff : t1; // Clamp-to-max
    t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero

    t1 |= t2;                              // Re-insert sign bit

    return (uint16_t)t1;
}

void UpdateMemoryFromInput() {
    if (currentInput.empty() || currentInput == L"-") return;

    try {
        if (currentDataType >= DT_HALF) { // Floating point
            double val = std::stod(currentInput);
            double storedVal = 0;

            if (currentDataType == DT_HALF) {
                calcMem.u16 = FloatToHalf((float)val);
                storedVal = HalfToFloat(calcMem.u16);
            }
            else if (currentDataType == DT_FLOAT) {
                calcMem.f32 = (float)val;
                storedVal = calcMem.f32;
            }
            else if (currentDataType == DT_DOUBLE) {
                calcMem.f64 = val;
                storedVal = calcMem.f64;
            }

            // Check if typed value lost precision when stored
            if (val != storedVal && currentBase == 10) {
                showPrecisionWarning = true;
                std::wstringstream ws;
                ws << std::defaultfloat << std::setprecision(10) << storedVal;
                exactDecimalInput = ws.str();
            }
            else {
                showPrecisionWarning = false;
            }
        }
        else { // Integer

            showPrecisionWarning = false;

            uint64_t val = 0;
            if (currentBase == 10) {
                val = std::stoull(currentInput);
            }
            else {
                std::wstring cleanInput = currentInput;
                if (cleanInput.find(L"0x") == 0) cleanInput = cleanInput.substr(2);
                else if (cleanInput.find(L"o") == 0) cleanInput = cleanInput.substr(1);
                else if (cleanInput.find(L"b") == 0) cleanInput = cleanInput.substr(1);

                val = std::stoull(cleanInput, nullptr, currentBase);
            }

            calcMem.u64 = val;
        }
    }
    catch (const std::exception&) {
        // Catch std::out_of_range and std::invalid_argument.
    }
}

std::wstring AddSeparators(std::wstring str, int base, bool isFloat) {
    if (str.empty() || str == L"0") return str;

    std::wstring prefix = L"";
    if (str[0] == L'-') { prefix = L"-"; str = str.substr(1); }
    else if (base == 16 && str.find(L"0x") == 0) { prefix = L"0x"; str = str.substr(2); }
    else if (base == 8 && str.find(L"o") == 0) { prefix = L"o"; str = str.substr(1); }
    else if (base == 2 && str.find(L"b") == 0) { prefix = L"b"; str = str.substr(1); }

    std::wstring intPart = str;
    std::wstring fracPart = L"";

    if (isFloat) {
        size_t dotPos = str.find(L'.');
        if (dotPos != std::wstring::npos) {
            intPart = str.substr(0, dotPos);
            fracPart = str.substr(dotPos);
        }
    }

    std::wstring result = L"";
    int groupSize = (base == 16 || base == 2) ? 4 : 3; // Hex/Bin - 4, Dec/Oct - 3
    wchar_t sepChar = (base == 10) ? L',' : L' ';

    int count = 0;
    for (int i = (int)intPart.length() - 1; i >= 0; i--) {
        result = intPart[i] + result;
        count++;
        if (count == groupSize && i > 0) {
            result = sepChar + result;
            count = 0;
        }
    }

    return prefix + result + fracPart;
}

// Formats calcMem back into currentInput & szCurrent
void UpdateDisplay() {
    if (currentMode == MODE_BASIC) {
        if (useSeparators) wcscpy_s(szCurrent, AddSeparators(currentInput, 10, true).c_str());
        else wcscpy_s(szCurrent, currentInput.c_str());
        InvalidateRect(hDisplay, NULL, TRUE);
        return;
    }

    std::wstringstream ss;

    if (currentDataType >= DT_HALF) {
        double val = 0;
        if (currentDataType == DT_HALF) val = HalfToFloat(calcMem.u16);
        else if (currentDataType == DT_FLOAT) val = calcMem.f32;
        else if (currentDataType == DT_DOUBLE) val = calcMem.f64;
        ss << val;
    }
    else {
        uint64_t val = 0;
        switch (currentDataType) {
        case DT_INT8:   val = (uint64_t)(int64_t)calcMem.i8; break;
        case DT_UINT8:  val = calcMem.u8; break;
        case DT_INT16:  val = (uint64_t)(int64_t)calcMem.i16; break;
        case DT_UINT16: val = calcMem.u16; break;
        case DT_INT32:  val = (uint64_t)(int64_t)calcMem.i32; break;
        case DT_UINT32: val = calcMem.u32; break;
        case DT_INT64:  val = (uint64_t)calcMem.i64; break;
        case DT_UINT64: val = calcMem.u64; break;
        }

        if (currentBase == 10) {
            bool isSigned = (currentDataType == DT_INT8 || currentDataType == DT_INT16 || currentDataType == DT_INT32 || currentDataType == DT_INT64);
            if (isSigned) ss << (int64_t)val;
            else ss << val;
        }
        else if (currentBase == 16) ss << L"0x" << std::hex << std::uppercase << val;
        else if (currentBase == 8) ss << L"o" << std::oct << val;
        else if (currentBase == 2) {
            std::wstring binStr = std::bitset<64>(val).to_string<wchar_t>();
            size_t firstOne = binStr.find(L'1');
            if (firstOne != std::wstring::npos) ss << L"b" << binStr.substr(firstOne);
            else ss << L"b0";
        }
    }

    std::wstring generatedStr = ss.str();

    if (currentBase == 10 && !newNumber) {
        if (useSeparators) wcscpy_s(szCurrent, AddSeparators(currentInput, currentBase, currentDataType >= DT_HALF).c_str());
        else wcscpy_s(szCurrent, currentInput.c_str());
    }
    else {
        currentInput = generatedStr;
        if (useSeparators) wcscpy_s(szCurrent, AddSeparators(currentInput, currentBase, currentDataType >= DT_HALF).c_str());
        else wcscpy_s(szCurrent, currentInput.c_str());
    }

    InvalidateRect(hDisplay, NULL, TRUE);
    if (hBitDisplay) InvalidateRect(hBitDisplay, NULL, TRUE);
}

std::wstring FormatDouble(double d) {
    wchar_t buf[64];
    swprintf_s(buf, L"%g", d);
    return std::wstring(buf);
}

double GetParsedInputValue() {
    if (currentMode == MODE_PROGRAMMER) {
        if (currentDataType >= DT_HALF) { // Float Types
            if (currentDataType == DT_HALF) return HalfToFloat(calcMem.u16);
            if (currentDataType == DT_FLOAT) return calcMem.f32;
            return calcMem.f64;
        }
        else { // Integer Types
            bool isSigned = (currentDataType == DT_INT8 || currentDataType == DT_INT16 || currentDataType == DT_INT32 || currentDataType == DT_INT64);
            if (isSigned) return (double)calcMem.i64;
            return (double)calcMem.u64;
        }
    }

    // Basic Mode
    try { return std::stod(currentInput); }
    catch (...) { return 0.0; }
}

void UpdateButtonStates(HWND hWnd) {

    if (currentMode == MODE_BASIC) {
        for (int i = 0; i < 18; i++) EnableWindow(hBtns[i], TRUE);
        return;
    }

    bool isFloat = (currentDataType >= DT_HALF);

    HMENU hMenu = GetMenu(hWnd);
    UINT menuState = isFloat ? (MF_BYCOMMAND | MF_GRAYED) : (MF_BYCOMMAND | MF_ENABLED);

    EnableMenuItem(hMenu, IDM_BASE_HEX, menuState);
    EnableMenuItem(hMenu, IDM_BASE_OCT, menuState);
    EnableMenuItem(hMenu, IDM_BASE_BIN, menuState);

    // Programmer Mode
    bool enableHex = (currentBase == 16);
    bool enableDec = (currentBase == 10 || currentBase == 16);
    bool enableOct = (currentBase == 8 || currentBase == 10 || currentBase == 16);

    // Hex
    for (int i = 0; i < 6; i++) {
        EnableWindow(hHexBtns[i], enableHex);
    }

    // Decimmal and octal
    EnableWindow(hBtns[9], enableOct);  // '2'
    EnableWindow(hBtns[10], enableOct); // '3'
    EnableWindow(hBtns[4], enableOct);  // '4'
    EnableWindow(hBtns[5], enableOct);  // '5'
    EnableWindow(hBtns[6], enableOct);  // '6'
    EnableWindow(hBtns[0], enableOct);  // '7'

    EnableWindow(hBtns[1], enableDec);  // '8'
    EnableWindow(hBtns[2], enableDec);  // '9'

    // Dot (Index 14) - only in floating point
    bool enableDot = (currentDataType >= DT_HALF);
    EnableWindow(hBtns[14], enableDot);
}

void CopyToClipboard(HWND hWnd, const std::wstring& text) {
    if (!OpenClipboard(hWnd)) return;
    EmptyClipboard();

    // Allocate global memory for the string
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
    if (hMem) {
        wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
        wcscpy_s(pMem, text.length() + 1, text.c_str());
        GlobalUnlock(hMem);
        SetClipboardData(CF_UNICODETEXT, hMem);
    }
    CloseClipboard();
}

void HandlePaste(HWND hWnd) {
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return;
    if (!OpenClipboard(hWnd)) return;

    HGLOBAL hMem = GetClipboardData(CF_UNICODETEXT);
    if (hMem) {
        wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
        if (pMem) {
            std::wstring pasted(pMem);
            GlobalUnlock(hMem);

            std::wstring clean = L"";
            for (wchar_t c : pasted) {
                if (c != L' ' && c != L',' && c != L'\r' && c != L'\n') clean += c;
            }

            if (!clean.empty()) {
                try {
                    if (currentMode == MODE_PROGRAMMER && currentDataType < DT_HALF) {
                        int base = currentBase;
                        std::wstring testClean = clean;
                        if (testClean.find(L"0x") == 0) { testClean = testClean.substr(2); base = 16; }
                        else if (testClean.find(L"o") == 0) { testClean = testClean.substr(1); base = 8; }
                        else if (testClean.find(L"b") == 0) { testClean = testClean.substr(1); base = 2; }

                        std::stoull(testClean, nullptr, base);
                    }
                    else {
                        std::stod(clean);
                    }

                    currentInput = clean;
                    newNumber = true;

                    if (currentMode == MODE_PROGRAMMER) UpdateMemoryFromInput();
                    UpdateDisplay();

                }
                catch (...) {
                }
            }
        }
    }
    CloseClipboard();
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
        if (currentMode == MODE_PROGRAMMER && currentDataType < DT_HALF) return;

        if (newNumber) { currentInput = L"0."; newNumber = false; }
        else if (currentInput.find(L'.') == std::wstring::npos) currentInput += L".";
    }
    else if (id == IDC_BTN_BS) {
        if (!newNumber && currentInput.length() > 0) {
            currentInput.pop_back();

            if (currentMode == MODE_PROGRAMMER) {
                if (currentInput == L"0x" || currentInput == L"o" || currentInput == L"b" || currentInput == L"-" || currentInput.empty()) {
                    currentInput = L"0";
                }
            }
            else {
                if (currentInput.empty() || currentInput == L"-") currentInput = L"0";
            }
        }
    }
    else if (id == IDC_BTN_CLEAR || id == IDM_CLEAR) {
        currentInput = L"0";
        historyString = L"";
        previousValue = 0;
        currentOp = 0;
        newNumber = true;
        showPrecisionWarning = false;
    }
    else if (id >= IDC_BTN_ADD && id <= IDC_BTN_DIV) {
        if (!newNumber || currentOp == 0) {
            if (currentOp != 0) {
                double currentValue = GetParsedInputValue();
                if (currentOp == IDC_BTN_ADD) previousValue += currentValue;
                if (currentOp == IDC_BTN_SUB) previousValue -= currentValue;
                if (currentOp == IDC_BTN_MUL) previousValue *= currentValue;
                if (currentOp == IDC_BTN_DIV && currentValue != 0) previousValue /= currentValue;
            }
            else {
                //previousValue = std::stod(currentInput); - Results in crash
                previousValue = GetParsedInputValue();
            }
        }
        currentOp = id;
        newNumber = true;
        wchar_t opChar = (id == IDC_BTN_ADD) ? L'+' : (id == IDC_BTN_SUB) ? L'-' : (id == IDC_BTN_MUL) ? L'*' : L'/';
        if (currentMode == MODE_PROGRAMMER) {
            if (currentDataType >= DT_HALF) {
                if (currentDataType == DT_HALF) calcMem.u16 = FloatToHalf((float)previousValue);
                else if (currentDataType == DT_FLOAT) calcMem.f32 = (float)previousValue;
                else calcMem.f64 = previousValue;
            }
            else {
                calcMem.u64 = (uint64_t)previousValue;
            }
            UpdateDisplay();
            historyString = currentInput + L" " + opChar;
        }
        else {
            currentInput = FormatDouble(previousValue);
            historyString = currentInput + L" " + opChar;
        }
    }
    else if (id == IDC_BTN_EQ) {
        if (currentOp != 0) {

            newNumber = true;
            double currentValue = GetParsedInputValue();
            if (currentOp == IDC_BTN_ADD) previousValue += currentValue;
            if (currentOp == IDC_BTN_SUB) previousValue -= currentValue;
            if (currentOp == IDC_BTN_MUL) previousValue *= currentValue;
            if (currentOp == IDC_BTN_DIV && currentValue != 0) previousValue /= currentValue;
            if (currentMode == MODE_PROGRAMMER) {
                if (currentDataType >= DT_HALF) {
                    if (currentDataType == DT_HALF) calcMem.u16 = FloatToHalf((float)previousValue);
                    else if (currentDataType == DT_FLOAT) calcMem.f32 = (float)previousValue;
                    else calcMem.f64 = previousValue;
                }
                else {
                    calcMem.u64 = (uint64_t)previousValue;
                }
                UpdateDisplay();
            }
            else {
                currentInput = FormatDouble(previousValue);
            }
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
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

    WNDCLASSEXW wcexBit = { 0 };
    wcexBit.cbSize = sizeof(WNDCLASSEX);
    wcexBit.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcexBit.lpfnWndProc = CustomBitDisplayWndProc;
    wcexBit.hInstance = hInstance;
    wcexBit.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcexBit.lpszClassName = L"CustomBitDisplay";
    RegisterClassExW(&wcexBit);

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
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

        isAlwaysOnTop = GetPrivateProfileIntW(L"Settings", L"AlwaysOnTop", 0, szIniPath) != 0;

        useSeparators = GetPrivateProfileIntW(L"Settings", L"Separators", 0, szIniPath) != 0;
        CheckMenuItem(hMenu, IDM_SEPARATORS, useSeparators ? MF_CHECKED : MF_UNCHECKED);

        SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);

        if (isAlwaysOnTop) {
            CheckMenuItem(hMenu, IDM_ALWAYSONTOP, MF_CHECKED);
            SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }

        hDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"CustomDisplay", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            0, 0, 0, 0, hWnd, (HMENU)IDC_DISPLAY, GetModuleHandle(NULL), NULL);

        hBitDisplay = CreateWindowExW(WS_EX_CLIENTEDGE, L"CustomBitDisplay", L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            0, 0, 0, 0, hWnd, (HMENU)IDC_BITDISPLAY, GetModuleHandle(NULL), NULL);

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

        const wchar_t* dataTypes[] = {
            L"Int 8-bit", L"UInt 8-bit", L"Int 16-bit", L"UInt 16-bit",
            L"Int 32-bit", L"UInt 32-bit", L"Int 64-bit", L"UInt 64-bit",
            L"Float 16-bit (Half)", L"Float 32-bit", L"Float 64-bit (Double)"
        };
        for (const wchar_t* type : dataTypes) {
            SendMessage(hComboType, CB_ADDSTRING, 0, (LPARAM)type);
        }

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

        currentDataType = (DataType)savedDataType;

        int savedBase = GetPrivateProfileIntW(L"Settings", L"Base", 1, szIniPath);
        SendMessage(hRadioBases[savedBase], BM_SETCHECK, BST_CHECKED, 0);

        CheckMenuRadioItem(hMenu, IDM_BASE_HEX, IDM_BASE_BIN, IDM_BASE_HEX + savedBase, MF_BYCOMMAND);

        if (savedBase == 0) currentBase = 16;
        else if (savedBase == 1) currentBase = 10;
        else if (savedBase == 2) currentBase = 8;
        else if (savedBase == 3) currentBase = 2;

        UpdateButtonStates(hWnd);

        return 0;
        }

    case WM_SIZE: {
        int cx = LOWORD(lParam);
        int cy = HIWORD(lParam);
        int gap = 5;

        int displayHeight = (cy / 4);

        int showCmd = (currentMode == MODE_PROGRAMMER) ? SW_SHOW : SW_HIDE;
        ShowWindow(hComboType, showCmd);
        ShowWindow(hBitDisplay, showCmd);
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
            hdwp = DeferWindowPos(hdwp, hBitDisplay, NULL, gap, displayHeight + gap, cx - (2 * gap), bitDisplayHeight, SWP_NOZORDER);

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
        if (g_hFontWarning) DeleteObject(g_hFontWarning);
        g_hFontCurrent = CreateFontW(displayHeight * 2 / 4, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontHistory = CreateFontW(displayHeight / 4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontWarning = CreateFontW(displayHeight / 4, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        InvalidateRect(hWnd, NULL, TRUE);
        InvalidateRect(hDisplay, NULL, TRUE);

        return 0;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int wmEvent = HIWORD(wParam);

            if (wmId >= IDC_BTN_0 && wmId <= IDC_BTN_DOT) {
                HandleButtonCommand(wmId);

                if (currentMode == MODE_PROGRAMMER) {
                    UpdateMemoryFromInput();
                    UpdateDisplay();
                }
                else {
                    InvalidateRect(hDisplay, NULL, TRUE);
                }
                SetFocus(hWnd);
                return 0;
            }

            if (wmId >= IDC_BTN_A && wmId <= IDC_BTN_F) {
                if (currentMode == MODE_PROGRAMMER && currentBase == 16) {
                    wchar_t hexChar = L'A' + (wmId - IDC_BTN_A);
                    if (newNumber) {
                        currentInput = std::wstring(1, hexChar);
                        newNumber = false;
                    }
                    else {
                        if (currentInput == L"0") currentInput = std::wstring(1, hexChar);
                        else currentInput += hexChar;
                    }
                    UpdateMemoryFromInput();
                    UpdateDisplay();
                }
                SetFocus(hWnd);
                return 0;
            }
            if (wmId == IDC_COMBO_TYPE && wmEvent == CBN_SELCHANGE) {
                currentDataType = (DataType)SendMessage(hComboType, CB_GETCURSEL, 0, 0);

                int showBases = (currentDataType >= DT_HALF) ? SW_HIDE : SW_SHOW;
                for (int i = 0; i < 4; i++) ShowWindow(hRadioBases[i], showBases);

                if (currentDataType >= DT_HALF) {
                    SendMessage(hWnd, WM_COMMAND, IDC_RADIO_DEC, 0);
                }

                newNumber = true;
                UpdateMemoryFromInput();
                UpdateDisplay();

                UpdateButtonStates(hWnd);

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

                UpdateButtonStates(hWnd);

                break;
            case IDM_PROGRAMMER:
                currentMode = MODE_PROGRAMMER;
                CheckMenuItem(GetMenu(hWnd), IDM_PROGRAMMER, MF_CHECKED);
                CheckMenuItem(GetMenu(hWnd), IDM_BASIC, MF_UNCHECKED);

                GetClientRect(hWnd, &rc);
                SendMessage(hWnd, WM_SIZE, SIZE_RESTORED, MAKELPARAM(rc.right, rc.bottom));

                UpdateButtonStates(hWnd);

                break;
            case IDC_RADIO_HEX:
            case IDM_BASE_HEX:
                currentBase = 16;
                newNumber = true;
                CheckRadioButton(hWnd, IDC_RADIO_HEX, IDC_RADIO_BIN, IDC_RADIO_HEX);
                CheckMenuRadioItem(GetMenu(hWnd), IDM_BASE_HEX, IDM_BASE_BIN, IDM_BASE_HEX, MF_BYCOMMAND);
                UpdateMemoryFromInput();
                UpdateDisplay();
                UpdateButtonStates(hWnd);
                break;
            case IDC_RADIO_DEC:
            case IDM_BASE_DEC:
                currentBase = 10;
                newNumber = true;
                CheckRadioButton(hWnd, IDC_RADIO_HEX, IDC_RADIO_BIN, IDC_RADIO_DEC);
                CheckMenuRadioItem(GetMenu(hWnd), IDM_BASE_HEX, IDM_BASE_BIN, IDM_BASE_DEC, MF_BYCOMMAND);
                UpdateMemoryFromInput();
                UpdateDisplay();
                UpdateButtonStates(hWnd);
                break;
            case IDC_RADIO_OCT:
            case IDM_BASE_OCT:
                currentBase = 8;
                newNumber = true;
                CheckRadioButton(hWnd, IDC_RADIO_HEX, IDC_RADIO_BIN, IDC_RADIO_OCT);
                CheckMenuRadioItem(GetMenu(hWnd), IDM_BASE_HEX, IDM_BASE_BIN, IDM_BASE_OCT, MF_BYCOMMAND);
                UpdateMemoryFromInput();
                UpdateDisplay();
                UpdateButtonStates(hWnd);
                break;
            case IDC_RADIO_BIN:
            case IDM_BASE_BIN:
                currentBase = 2;
                newNumber = true;
                CheckRadioButton(hWnd, IDC_RADIO_HEX, IDC_RADIO_BIN, IDC_RADIO_BIN);
                CheckMenuRadioItem(GetMenu(hWnd), IDM_BASE_HEX, IDM_BASE_BIN, IDM_BASE_BIN, MF_BYCOMMAND);
                UpdateMemoryFromInput();
                UpdateDisplay();
                UpdateButtonStates(hWnd);
                break;
            case IDM_COPY:
                CopyToClipboard(hWnd, currentInput);
                break;
            case IDM_PASTE:
                HandlePaste(hWnd);
                break;
            case IDM_CLEAR:
                HandleButtonCommand(IDC_BTN_CLEAR);
                InvalidateRect(hDisplay, NULL, TRUE);
                break;
            case IDM_SEPARATORS:
                useSeparators = !useSeparators;
                CheckMenuItem(GetMenu(hWnd), IDM_SEPARATORS, useSeparators ? MF_CHECKED : MF_UNCHECKED);
                UpdateDisplay();
                break;
            case IDM_ALWAYSONTOP:
                isAlwaysOnTop = !isAlwaysOnTop;
                CheckMenuItem(GetMenu(hWnd), IDM_ALWAYSONTOP, isAlwaysOnTop ? MF_CHECKED : MF_UNCHECKED);

                SetWindowPos(hWnd, isAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

                if (!isAlwaysOnTop) {
                    SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
                }
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
        else if ((wParam >= 'A' && wParam <= 'F') && currentMode == MODE_PROGRAMMER) PostMessage(hWnd, WM_COMMAND, IDC_BTN_A + (wParam - 'A'), 0);
        else if ((wParam >= 'a' && wParam <= 'f') && currentMode == MODE_PROGRAMMER) PostMessage(hWnd, WM_COMMAND, IDC_BTN_A + (wParam - 'a'), 0);
        else if (wParam == '+') PostMessage(hWnd, WM_COMMAND, IDC_BTN_ADD, 0);
        else if (wParam == '-') PostMessage(hWnd, WM_COMMAND, IDC_BTN_SUB, 0);
        else if (wParam == '*') PostMessage(hWnd, WM_COMMAND, IDC_BTN_MUL, 0);
        else if (wParam == '/') PostMessage(hWnd, WM_COMMAND, IDC_BTN_DIV, 0);
        else if (wParam == '.' || wParam == ',') PostMessage(hWnd, WM_COMMAND, IDC_BTN_DOT, 0);
        else if (wParam == '=') PostMessage(hWnd, WM_COMMAND, IDC_BTN_EQ, 0);
        break;

    case WM_BIT_FLIPPED:
        newNumber = true;
        UpdateDisplay();
        return 0;

    case WM_ACTIVATE:
        if (isAlwaysOnTop) {
            if (LOWORD(wParam) == WA_INACTIVE) {
                SetLayeredWindowAttributes(hWnd, 0, 0.7 * 255, LWA_ALPHA);
            }
            else {
                SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA);
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
        if (currentMode == MODE_BASIC) {
            lpMMI->ptMinTrackSize.x = 320;
            lpMMI->ptMinTrackSize.y = 400;
        }
        else {
            lpMMI->ptMinTrackSize.x = 320;
            lpMMI->ptMinTrackSize.y = 580;
        }
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

        WritePrivateProfileStringW(L"Settings", L"Separators", useSeparators ? L"1" : L"0", szIniPath);

        WritePrivateProfileStringW(L"Settings", L"AlwaysOnTop", isAlwaysOnTop ? L"1" : L"0", szIniPath);

        int selectedBase = 1; // Default to Decimal
        if (currentBase == 16) selectedBase = 0;
        else if (currentBase == 10) selectedBase = 1;
        else if (currentBase == 8) selectedBase = 2;
        else if (currentBase == 2) selectedBase = 3;

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
        rcCurrent.bottom = rc.bottom - 20;
        rcCurrent.right -= 5;

        // Precision warning
        RECT rcWarn = rc;
        rcWarn.top = rc.bottom - 20;
        rcWarn.bottom = rc.bottom - 2;
        rcWarn.right -= 5;

        SetBkMode(hdc, TRANSPARENT);

        if (g_hFontHistory) SelectObject(hdc, g_hFontHistory);
        SetTextColor(hdc, RGB(100, 100, 100));
        DrawTextW(hdc, szHistory, -1, &rcHistory, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

        if (g_hFontCurrent) SelectObject(hdc, g_hFontCurrent);
        SetTextColor(hdc, RGB(0, 0, 0));
        DrawTextW(hdc, szCurrent, -1, &rcCurrent, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

        if (showPrecisionWarning && currentMode == MODE_PROGRAMMER && currentDataType >= DT_HALF && currentBase == 10) {
            if(g_hFontWarning) SelectObject(hdc, g_hFontWarning);

            SetTextColor(hdc, RGB(220, 0, 0)); // Pure Red Text

            std::wstring warnMsg = L"Stored value: " + exactDecimalInput;
            DrawTextW(hdc, warnMsg.c_str(), -1, &rcWarn, DT_RIGHT | DT_SINGLELINE | DT_BOTTOM);

        }

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

LRESULT CALLBACK CustomBitDisplayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static int currentHover = -1;

    auto GetGridLayout = [](int& activeBits, int& cols, int& rows) {
        if (currentDataType <= DT_UINT8) { activeBits = 8; cols = 8; rows = 1; }
        else if (currentDataType <= DT_UINT16 || currentDataType == DT_HALF) { activeBits = 16; cols = 16; rows = 1; }
        else if (currentDataType <= DT_UINT32 || currentDataType == DT_FLOAT) { activeBits = 32; cols = 16; rows = 2; }
        else { activeBits = 64; cols = 16; rows = 4; }
        };

    switch (message) {
    case WM_ERASEBKGND:
        return 1;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));

        int activeBits, cols, rows;
        GetGridLayout(activeBits, cols, rows);

        float cellW = (float)rc.right / cols;
        float cellH = (float)rc.bottom / rows;

        int fontSizeLarge = max(8, (int)(min(cellH, cellW) * 0.9f));
        HFONT hFontLarge = CreateFontW(fontSizeLarge, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

        int fontSizeSmall = max(6, (int)(min(cellH, cellW) * 0.35f));
        HFONT hFontSmall = CreateFontW(fontSizeSmall, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");

        SetBkMode(hdcMem, TRANSPARENT);

        // Draw bits
        for (int i = 0; i < activeBits; i++) {
            int bitIndex = (activeBits - 1) - i;
            int row = i / cols;
            int col = i % cols;

            int leftEdge = (int)(col * cellW);
            int topEdge = (int)(row * cellH);
            int rightEdge = (col == cols - 1) ? rc.right : (int)((col + 1) * cellW);
            int bottomEdge = (row == rows - 1) ? rc.bottom : (int)((row + 1) * cellH);

            RECT cellRc = { leftEdge, topEdge, rightEdge, bottomEdge };
            RECT mainRc = { cellRc.left, cellRc.top, cellRc.right, cellRc.bottom - (int)(cellH * 0.4f) };
            RECT idxRc = { cellRc.left, cellRc.bottom - (int)(cellH * 0.35f), cellRc.right, cellRc.bottom };

            COLORREF bgCol = RGB(240, 240, 255);
            COLORREF ordCol = RGB(150, 150, 160);

            if (currentDataType >= DT_HALF) {
                int expBits = (currentDataType == DT_HALF) ? 5 : (currentDataType == DT_FLOAT) ? 8 : 11;
                int mantBits = (currentDataType == DT_HALF) ? 10 : (currentDataType == DT_FLOAT) ? 23 : 52;

                if (bitIndex == expBits + mantBits) {
                    bgCol = RGB(255, 220, 220); ordCol = RGB(200, 100, 100);
                }
                else if (bitIndex >= mantBits && bitIndex < expBits + mantBits) {
                    bgCol = RGB(220, 255, 220); ordCol = RGB(100, 180, 100);
                }
                else if (bitIndex < mantBits) {
                    bgCol = RGB(220, 220, 255); ordCol = RGB(120, 120, 200);
                }
            }

            if (bitIndex == currentHover) {
                bgCol = RGB(max(0, GetRValue(bgCol) - 40), max(0, GetGValue(bgCol) - 40), max(0, GetBValue(bgCol) - 40));
            }

            HBRUSH hBr = CreateSolidBrush(bgCol);
            FillRect(hdcMem, &cellRc, hBr);
            DeleteObject(hBr);

            SelectObject(hdcMem, hFontSmall);
            SetTextColor(hdcMem, ordCol);
            std::wstring sIdx = std::to_wstring(bitIndex);
            DrawTextW(hdcMem, sIdx.c_str(), -1, &idxRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            int bitVal = (calcMem.u64 >> bitIndex) & 1;
            SelectObject(hdcMem, hFontLarge);
            COLORREF digitCol = (bitIndex == currentHover) ? RGB(0, 0, 255) : (bitVal ? RGB(0, 0, 0) : RGB(170, 170, 170));
            SetTextColor(hdcMem, digitCol);
            std::wstring sVal = std::to_wstring(bitVal);
            DrawTextW(hdcMem, sVal.c_str(), -1, &mainRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        // Draw Tooltip
        if (currentHover != -1) {
            std::wstring tipText;
            if (currentDataType < DT_HALF) {
                uint64_t weight = (1ULL << currentHover);
                tipText = L"Bit " + std::to_wstring(currentHover) + L"\nDecimal Weight: " + std::to_wstring(weight);
            }
            else {
                int expBits = (currentDataType == DT_HALF) ? 5 : (currentDataType == DT_FLOAT) ? 8 : 11;
                int mantBits = (currentDataType == DT_HALF) ? 10 : (currentDataType == DT_FLOAT) ? 23 : 52;

                if (currentHover == expBits + mantBits) {
                    tipText = L"Sign Bit\n(0 = Positive, 1 = Negative)";
                }
                else if (currentHover >= mantBits) {
                    int expIdx = currentHover - mantBits;
                    uint64_t weight = (1ULL << expIdx);
                    tipText = L"Exponent Bit " + std::to_wstring(expIdx) + L"\nWeight: " + std::to_wstring(weight);
                }
                else {
                    int mantIdx = mantBits - currentHover;
                    tipText = L"Mantissa Bit " + std::to_wstring(currentHover) + L"\nFractional Weight: 2^-" + std::to_wstring(mantIdx);
                }
            }

            HFONT hTipFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
            SelectObject(hdcMem, hTipFont);

            RECT textRc = { 0, 0, 0, 0 };
            DrawTextW(hdcMem, tipText.c_str(), -1, &textRc, DT_CALCRECT | DT_LEFT);

            int pad = 5;
            int tipW = (textRc.right - textRc.left) + (pad * 2);
            int tipH = (textRc.bottom - textRc.top) + (pad * 2);

            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);

            int tipX = pt.x + 15;
            int tipY = pt.y + 15;

            if (tipX + tipW > rc.right) tipX = pt.x - tipW - 5;
            if (tipY + tipH > rc.bottom) tipY = pt.y - tipH - 5;
            if (tipX < 0) tipX = 0;
            if (tipY < 0) tipY = 0;

            RECT bgRc = { tipX, tipY, tipX + tipW, tipY + tipH };
            RECT drawRc = { tipX + pad, tipY + pad, tipX + tipW - pad, tipY + tipH - pad };

            HBRUSH hTipBr = CreateSolidBrush(RGB(255, 255, 225));
            HPEN hTipPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
            SelectObject(hdcMem, hTipBr);
            SelectObject(hdcMem, hTipPen);

            Rectangle(hdcMem, bgRc.left, bgRc.top, bgRc.right, bgRc.bottom);

            SetTextColor(hdcMem, RGB(0, 0, 0));
            DrawTextW(hdcMem, tipText.c_str(), -1, &drawRc, DT_LEFT);

            DeleteObject(hTipBr);
            DeleteObject(hTipPen);
            DeleteObject(hTipFont);
        }

        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        DeleteObject(hFontLarge);
        DeleteObject(hFontSmall);
        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_MOUSEMOVE: {
        RECT rc;
        GetClientRect(hWnd, &rc);

        int activeBits, cols, rows;
        GetGridLayout(activeBits, cols, rows);

        float cellW = (float)rc.right / cols;
        float cellH = (float)rc.bottom / rows;

        int col = (int)(LOWORD(lParam) / cellW);
        int row = (int)(HIWORD(lParam) / cellH);

        if (col >= cols) col = cols - 1;
        if (row >= rows) row = rows - 1;

        if (col >= 0 && col < cols && row >= 0 && row < rows) {
            int bitIndex = (activeBits - 1) - (row * cols + col);
            if (bitIndex >= 0 && bitIndex < activeBits) {
                currentHover = bitIndex;

                InvalidateRect(hWnd, NULL, FALSE);

                TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
                TrackMouseEvent(&tme);
            }
        }
        else if (currentHover != -1) {
            currentHover = -1;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_MOUSELEAVE:
        currentHover = -1;
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;

    case WM_LBUTTONDOWN:
        if (currentHover != -1) {
            calcMem.u64 ^= (1ULL << currentHover);
            PostMessage(GetParent(hWnd), WM_BIT_FLIPPED, 0, 0);
        }
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
