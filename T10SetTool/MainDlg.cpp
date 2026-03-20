// MainDlg.cpp  -  T10 reader parameter configuration tool
// Build: MSVC 2022, Win32 (x86), Unicode (/utf-8)

#include "MainDlg.h"
#include <windowsx.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

// ---------------------------------------------------------------
//  Global application context
// ---------------------------------------------------------------
static AppCtx g_ctx;

// DLL path (relative first, then absolute fallback)
static const wchar_t* k_DllRelPath = L"..\\dcrf32-10.3.4.2-Windows_x86\\bin\\dcrf32.dll";
static const wchar_t* k_DllAbsPath = L"D:\\WorkBuddyWork\\T10_Set_Rf\\dcrf32-10.3.4.2-Windows_x86\\bin\\dcrf32.dll";

// ---------------------------------------------------------------
//  Helper: append a timestamped log line to the log edit box
// ---------------------------------------------------------------
static void AppendLog(HWND hEdit, const wchar_t* msg)
{
    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_s(&tm_now, &now);
    wchar_t ts[32];
    swprintf_s(ts, L"[%02d:%02d:%02d] ", tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec);

    int len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)ts);
    len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
    len = GetWindowTextLengthW(hEdit);
    SendMessageW(hEdit, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageW(hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
    SendMessageW(hEdit, EM_SCROLLCARET, 0, 0);
}

static void LogFmt(HWND hDlg, const wchar_t* fmt, ...)
{
    wchar_t buf[512];
    va_list args;
    va_start(args, fmt);
    vswprintf_s(buf, fmt, args);
    va_end(args);
    AppendLog(GetDlgItem(hDlg, IDC_EDIT_LOG), buf);
}

// ---------------------------------------------------------------
//  Load dcrf32.dll and resolve function pointers
// ---------------------------------------------------------------
static bool LoadDll(HWND hDlg)
{
    if (g_ctx.hDll) return true;

    HMODULE hDll = LoadLibraryW(k_DllRelPath);
    if (!hDll) hDll = LoadLibraryW(L"dcrf32.dll");
    if (!hDll) hDll = LoadLibraryW(k_DllAbsPath);

    if (!hDll) {
        LogFmt(hDlg, L"[ERR] Cannot load dcrf32.dll (code=%lu)", GetLastError());
        MessageBoxW(hDlg,
            L"Cannot load dcrf32.dll!\r\nCheck DLL path and ensure x86 build.",
            L"Load Error", MB_ICONERROR | MB_OK);
        return false;
    }

#define GETPROC(name, type, member) \
    g_ctx.member = (type)GetProcAddress(hDll, name); \
    if (!g_ctx.member) { \
        LogFmt(hDlg, L"[ERR] Function not found: " L#name); \
        FreeLibrary(hDll); return false; \
    }

    GETPROC("dc_init",             PFN_dc_init,             pDcInit)
    GETPROC("dc_exit",             PFN_dc_exit,             pDcExit)
    GETPROC("dc_getver",           PFN_dc_getver,           pDcGetVer)
    GETPROC("dc_RfUserAttributes", PFN_dc_RfUserAttributes, pDcRfAttr)
#undef GETPROC

    g_ctx.hDll = hDll;
    LogFmt(hDlg, L"[OK] dcrf32.dll loaded successfully");
    return true;
}

// ---------------------------------------------------------------
//  Enable / disable controls based on connection state
// ---------------------------------------------------------------
static void UpdateControlState(HWND hDlg)
{
    bool connected = g_ctx.IsConnected();
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_CONNECT),    !connected);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_DISCONNECT),  connected);
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_CONNTYPE),  !connected);
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_COMPORT),   !connected);
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BAUD),      !connected);

    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_RF_OP),     connected);
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_RF_RATE),   connected);
    EnableWindow(GetDlgItem(hDlg, IDC_EDIT_WTX),        connected);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_RF_SET),      connected);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_RF_GET),      connected);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_RF_RESET),    connected);
    EnableWindow(GetDlgItem(hDlg, IDC_BTN_RF_PROBE),    connected);
}

static void UpdateComPortVisibility(HWND hDlg)
{
    int  sel      = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_CONNTYPE));
    bool isSerial = (sel == 1);
    bool conn     = g_ctx.IsConnected();
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_COMPORT), isSerial && !conn);
    EnableWindow(GetDlgItem(hDlg, IDC_COMBO_BAUD),    isSerial && !conn);
}

// ---------------------------------------------------------------
//  WM_INITDIALOG
// ---------------------------------------------------------------
static void OnInitDialog(HWND hDlg)
{
    // Set window title with version
    wchar_t title[256];
    swprintf_s(title, L"%hs v%hs", APP_PRODUCT_NAME, APP_VERSION_STRING);
    SetWindowTextW(hDlg, title);

    // Icon
    HICON hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_APPICON));
    SendMessageW(hDlg, WM_SETICON, ICON_BIG,   (LPARAM)hIcon);
    SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

    // Connection type
    HWND hConn = GetDlgItem(hDlg, IDC_COMBO_CONNTYPE);
    ComboBox_AddString(hConn, L"USB (port=100)");
    ComboBox_AddString(hConn, L"Serial (COM)");
    ComboBox_SetCurSel(hConn, 0);

    // COM port list
    HWND hCom = GetDlgItem(hDlg, IDC_COMBO_COMPORT);
    for (int i = 1; i <= 16; i++) {
        wchar_t s[16];
        swprintf_s(s, L"COM%d", i);
        ComboBox_AddString(hCom, s);
    }
    ComboBox_SetCurSel(hCom, 0);

    // Baud rates
    HWND hBaud = GetDlgItem(hDlg, IDC_COMBO_BAUD);
    const wchar_t* bauds[] = { L"9600", L"19200", L"38400", L"57600", L"115200", L"230400" };
    for (auto b : bauds) ComboBox_AddString(hBaud, b);
    ComboBox_SetCurSel(hBaud, 4);   // default 115200

    // RF operation type
    HWND hOp = GetDlgItem(hDlg, IDC_COMBO_RF_OP);
    ComboBox_AddString(hOp, L"type=0x01  Set RF Rate");
    ComboBox_AddString(hOp, L"type=0x02  Get RF Rate");
    ComboBox_AddString(hOp, L"type=0x03  Set WTX Count");
    ComboBox_AddString(hOp, L"type=0x04  Get WTX Count");
    ComboBox_SetCurSel(hOp, 0);

    // RF rate options
    HWND hRate = GetDlgItem(hDlg, IDC_COMBO_RF_RATE);
    ComboBox_AddString(hRate, L"106K  (0x00)");
    ComboBox_AddString(hRate, L"212K  (0x11)");
    ComboBox_AddString(hRate, L"424K  (0x33)");
    ComboBox_AddString(hRate, L"848K  (0x77)");
    ComboBox_SetCurSel(hRate, 0);

    // WTX default
    SetDlgItemTextW(hDlg, IDC_EDIT_WTX, L"10");

    UpdateControlState(hDlg);
    UpdateComPortVisibility(hDlg);
    LogFmt(hDlg, L"Ready. Select connection type and click [Connect].");
}

// ---------------------------------------------------------------
//  Connect device (IDC_BTN_CONNECT)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(179): [HANDLER] OnConnect() defined here")
static void OnConnect(HWND hDlg)
{
    if (!LoadDll(hDlg)) return;

    int   connSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_CONNTYPE));
    short port;
    int   baud = 0;

    if (connSel == 0) {
        port = 100;
        LogFmt(hDlg, L"Opening USB device (port=100)...");
    } else {
        int comIdx  = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_COMPORT));
        int baudIdx = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_BAUD));
        const int baudVals[] = { 9600, 19200, 38400, 57600, 115200, 230400 };
        port = (short)comIdx;
        baud = baudVals[baudIdx];
        LogFmt(hDlg, L"Opening COM%d @ %d bps...", comIdx + 1, baud);
    }

    HANDLE h = g_ctx.pDcInit(port, baud);
    if ((intptr_t)h < 0) {
        LogFmt(hDlg, L"[ERR] dc_init failed, ret=%lld", (long long)(intptr_t)h);
        MessageBoxW(hDlg,
            L"Failed to open device.\r\nCheck connection and port settings.",
            L"Connect Error", MB_ICONERROR | MB_OK);
        return;
    }

    g_ctx.hDevice = h;
    LogFmt(hDlg, L"[OK] Device opened, handle=0x%p", h);

    // Read firmware version
    unsigned char verBuf[128] = {};
    short ret = g_ctx.pDcGetVer(h, verBuf);
    if (ret == 0) {
        wchar_t wVer[256];
        MultiByteToWideChar(CP_ACP, 0, (char*)verBuf, -1, wVer, 256);
        SetDlgItemTextW(hDlg, IDC_EDIT_VERSION, wVer);
        LogFmt(hDlg, L"[OK] Firmware version: %s", wVer);
    } else {
        SetDlgItemTextW(hDlg, IDC_EDIT_VERSION, L"(read failed)");
        LogFmt(hDlg, L"[WARN] dc_getver ret=%d (device still usable)", ret);
    }

    UpdateControlState(hDlg);
}

// ---------------------------------------------------------------
//  Disconnect device (IDC_BTN_DISCONNECT)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(230): [HANDLER] OnDisconnect() defined here")
static void OnDisconnect(HWND hDlg)
{
    if (!g_ctx.IsConnected()) return;

    short ret = g_ctx.pDcExit(g_ctx.hDevice);
    LogFmt(hDlg, ret == 0 ? L"[OK] Device closed." : L"[WARN] dc_exit ret=%d");
    g_ctx.hDevice = (HANDLE)-1;
    SetDlgItemTextW(hDlg, IDC_EDIT_VERSION, L"");
    UpdateControlState(hDlg);
}

// ---------------------------------------------------------------
//  Map combo index -> dc_RfUserAttributes type value
// ---------------------------------------------------------------
static unsigned char OpSelToType(int sel, bool& isGet)
{
    static const unsigned char types[] = { 0x01, 0x02, 0x03, 0x04 };
    static const bool          gets[]  = { false, true, false, true };
    isGet = gets[sel];
    return types[sel];
}

static unsigned short RateSelToValue(int sel)
{
    static const unsigned short vals[] = { 0x00, 0x11, 0x33, 0x77 };
    return vals[sel];
}

static const wchar_t* RateValueToStr(unsigned short v)
{
    switch (v) {
        case 0x00: return L"106K (0x00)";
        case 0x11: return L"212K (0x11)";
        case 0x33: return L"424K (0x33)";
        case 0x77: return L"848K (0x77)";
        default:   return L"Unknown";
    }
}

// ---------------------------------------------------------------
//  Restore default RF attributes (IDC_BTN_RF_RESET, type=0x00)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(274): [HANDLER] OnRfReset() defined here")
static void OnRfReset(HWND hDlg)
{
    if (!g_ctx.IsConnected()) return;
    unsigned short dummy = 0;
    short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x00, &dummy);
    if (ret == 0)
        LogFmt(hDlg, L"[OK] RF attributes restored to device defaults.");
    else
        LogFmt(hDlg, L"[ERR] Restore default failed, ret=%d", ret);
}

// ---------------------------------------------------------------
//  Set RF attribute (IDC_BTN_RF_SET)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(289): [HANDLER] OnRfSet() defined here")
static void OnRfSet(HWND hDlg)
{
    if (!g_ctx.IsConnected()) return;

    int  opSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_RF_OP));
    bool isGet;
    unsigned char type = OpSelToType(opSel, isGet);

    if (isGet) {
        LogFmt(hDlg, L"[WARN] Current selection is GET. Use [Get] button.");
        return;
    }

    unsigned short value = 0;
    if (type == 0x01) {
        int rateSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_RF_RATE));
        value = RateSelToValue(rateSel);
        LogFmt(hDlg, L"Setting RF rate -> %s ...", RateValueToStr(value));
    } else if (type == 0x03) {
        wchar_t buf[32];
        GetDlgItemTextW(hDlg, IDC_EDIT_WTX, buf, 32);
        value = (unsigned short)_wtoi(buf);
        LogFmt(hDlg, L"Setting WTX count -> %u ...", (unsigned)value);
    }

    short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, type, &value);
    if (ret == 0)
        LogFmt(hDlg, L"[OK] Set OK (type=0x%02X, value=0x%04X)", type, value);
    else
        LogFmt(hDlg, L"[ERR] Set failed (type=0x%02X), ret=%d", type, ret);
}

// ---------------------------------------------------------------
//  Get RF attribute (IDC_BTN_RF_GET)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(325): [HANDLER] OnRfGet() defined here")
static void OnRfGet(HWND hDlg)
{
    if (!g_ctx.IsConnected()) return;

    int  opSel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBO_RF_OP));
    bool isGet;
    unsigned char type = OpSelToType(opSel, isGet);

    // Auto-switch to GET variant if SET was selected
    if (!isGet) type = (type == 0x01) ? (unsigned char)0x02 : (unsigned char)0x04;

    unsigned short value = 0;
    short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, type, &value);
    if (ret == 0) {
        if (type == 0x02) {
            LogFmt(hDlg, L"[OK] Current RF rate: %s (raw=0x%04X)", RateValueToStr(value), value);
            HWND hRate = GetDlgItem(hDlg, IDC_COMBO_RF_RATE);
            if      (value == 0x00) ComboBox_SetCurSel(hRate, 0);
            else if (value == 0x11) ComboBox_SetCurSel(hRate, 1);
            else if (value == 0x33) ComboBox_SetCurSel(hRate, 2);
            else if (value == 0x77) ComboBox_SetCurSel(hRate, 3);
        } else if (type == 0x04) {
            LogFmt(hDlg, L"[OK] Current WTX count: %u", (unsigned)value);
            wchar_t buf[16];
            swprintf_s(buf, L"%u", (unsigned)value);
            SetDlgItemTextW(hDlg, IDC_EDIT_WTX, buf);
        }
    } else {
        if (ret == -2)
            LogFmt(hDlg, L"[WARN] Get not supported by firmware (type=0x%02X, ret=-2). Try [Probe All] to check.", type);
        else
            LogFmt(hDlg, L"[ERR] Get failed (type=0x%02X), ret=%d", type, ret);
    }
}

// ---------------------------------------------------------------
//  Probe all dc_RfUserAttributes type values (IDC_BTN_RF_PROBE, 0x00~0x04)
// ---------------------------------------------------------------
#pragma message(__FILE__ "(364): [HANDLER] OnRfProbe() defined here")
static void OnRfProbe(HWND hDlg)
{
    if (!g_ctx.IsConnected()) return;
    LogFmt(hDlg, L"--- Probing dc_RfUserAttributes (type 0x00~0x04) ---");

    // type 0x00: restore default (write-only, just try and restore after)
    {
        unsigned short dummy = 0;
        short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x00, &dummy);
        LogFmt(hDlg, L"  type=0x00 (RestoreDefault): ret=%d %s",
               ret, ret == 0 ? L"[OK]" : (ret == -2 ? L"[not supported]" : L"[failed]"));
    }

    // type 0x01: set rate (set to 106K = 0x00)
    {
        unsigned short val = 0x00;
        short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x01, &val);
        LogFmt(hDlg, L"  type=0x01 (SetRate=106K):   ret=%d %s",
               ret, ret == 0 ? L"[OK]" : (ret == -2 ? L"[not supported]" : L"[failed]"));
    }

    // type 0x02: get rate
    {
        unsigned short val = 0;
        short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x02, &val);
        if (ret == 0)
            LogFmt(hDlg, L"  type=0x02 (GetRate):        ret=0 [OK]  value=0x%04X (%s)", val, RateValueToStr(val));
        else
            LogFmt(hDlg, L"  type=0x02 (GetRate):        ret=%d %s",
                   ret, ret == -2 ? L"[not supported by firmware]" : L"[failed]");
    }

    // type 0x03: set WTX count (set to 10)
    {
        unsigned short val = 10;
        short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x03, &val);
        LogFmt(hDlg, L"  type=0x03 (SetWTX=10):      ret=%d %s",
               ret, ret == 0 ? L"[OK]" : (ret == -2 ? L"[not supported]" : L"[failed]"));
    }

    // type 0x04: get WTX count
    {
        unsigned short val = 0;
        short ret = g_ctx.pDcRfAttr(g_ctx.hDevice, 0x04, &val);
        if (ret == 0)
            LogFmt(hDlg, L"  type=0x04 (GetWTX):         ret=0 [OK]  value=%u", (unsigned)val);
        else
            LogFmt(hDlg, L"  type=0x04 (GetWTX):         ret=%d %s",
                   ret, ret == -2 ? L"[not supported by firmware]" : L"[failed]");
    }

    LogFmt(hDlg, L"--- Probe done ---");
}

// ---------------------------------------------------------------
//  Dialog message procedure
// ---------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        OnInitDialog(hDlg);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        // Navigation hints: double-click message in Output window to jump here
        // Format: filepath(lineno): [UI-EVENT] ControlID -> HandlerName
        #pragma message(__FILE__ "(433): [UI-EVENT] IDC_BTN_CONNECT -> OnConnect()")
        case IDC_BTN_CONNECT:     OnConnect(hDlg);     break;

        #pragma message(__FILE__ "(436): [UI-EVENT] IDC_BTN_DISCONNECT -> OnDisconnect()")
        case IDC_BTN_DISCONNECT:  OnDisconnect(hDlg);  break;

        #pragma message(__FILE__ "(439): [UI-EVENT] IDC_BTN_RF_RESET -> OnRfReset()")
        case IDC_BTN_RF_RESET:    OnRfReset(hDlg);     break;

        #pragma message(__FILE__ "(442): [UI-EVENT] IDC_BTN_RF_SET -> OnRfSet()")
        case IDC_BTN_RF_SET:      OnRfSet(hDlg);       break;

        #pragma message(__FILE__ "(445): [UI-EVENT] IDC_BTN_RF_GET -> OnRfGet()")
        case IDC_BTN_RF_GET:      OnRfGet(hDlg);       break;

        #pragma message(__FILE__ "(448): [UI-EVENT] IDC_BTN_RF_PROBE -> OnRfProbe()")
        case IDC_BTN_RF_PROBE:    OnRfProbe(hDlg);     break;

        #pragma message(__FILE__ "(451): [UI-EVENT] IDC_BTN_CLEAR_LOG -> ClearLog")
        case IDC_BTN_CLEAR_LOG:
            SetDlgItemTextW(hDlg, IDC_EDIT_LOG, L"");
            break;

        #pragma message(__FILE__ "(456): [UI-EVENT] IDC_COMBO_CONNTYPE -> UpdateComPortVisibility()")
        case IDC_COMBO_CONNTYPE:
            if (HIWORD(wParam) == CBN_SELCHANGE)
                UpdateComPortVisibility(hDlg);
            break;

        case IDCANCEL:
            OnDisconnect(hDlg);
            EndDialog(hDlg, 0);
            break;
        }
        return (INT_PTR)TRUE;

    case WM_CLOSE:
        OnDisconnect(hDlg);
        EndDialog(hDlg, 0);
        return (INT_PTR)TRUE;

    case WM_DESTROY:
        if (g_ctx.hDll) {
            FreeLibrary(g_ctx.hDll);
            g_ctx.hDll = nullptr;
        }
        return (INT_PTR)FALSE;
    }
    return (INT_PTR)FALSE;
}
