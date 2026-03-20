#pragma once

#include <windows.h>
#include <commctrl.h>
#include "resource.h"

// dcrf32.dll function pointer types
typedef HANDLE (__stdcall *PFN_dc_init)(short port, int baud);
typedef short  (__stdcall *PFN_dc_exit)(HANDLE icdev);
typedef short  (__stdcall *PFN_dc_getver)(HANDLE icdev, unsigned char *sver);
typedef short  (__stdcall *PFN_dc_RfUserAttributes)(HANDLE icdev, unsigned char type, unsigned short *value);

// Dialog procedure
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Shared application context
struct AppCtx {
    HMODULE             hDll      = nullptr;
    PFN_dc_init         pDcInit   = nullptr;
    PFN_dc_exit         pDcExit   = nullptr;
    PFN_dc_getver       pDcGetVer = nullptr;
    PFN_dc_RfUserAttributes pDcRfAttr = nullptr;

    HANDLE  hDevice = (HANDLE)(intptr_t)-1;
    bool IsConnected() const { return (intptr_t)hDevice >= 0; }
};