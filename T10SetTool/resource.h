#ifndef RESOURCE_H
#define RESOURCE_H

// Version info
#define APP_VERSION_MAJOR               1
#define APP_VERSION_MINOR               0
#define APP_VERSION_PATCH               0
#define APP_VERSION_BUILD               1
#define APP_VERSION_STRING              "1.0.0.1"
#define APP_PRODUCT_NAME                "T10 Reader Configuration Tool"
#define APP_COMPANY_NAME                "Your Company"

// App icon
#define IDI_APPICON                     101

// Dialog
#define IDD_MAIN_DIALOG                 1000

// Connection group
#define IDC_COMBO_CONNTYPE              1001
#define IDC_COMBO_COMPORT               1002
#define IDC_COMBO_BAUD                  1003
#define IDC_BTN_CONNECT                 1004
#define IDC_BTN_DISCONNECT              1005
#define IDC_EDIT_VERSION                1006

// RF User Attributes group
#define IDC_COMBO_RF_OP                 2001
#define IDC_COMBO_RF_RATE               2002
#define IDC_EDIT_WTX                    2003
#define IDC_BTN_RF_RESET                2004
#define IDC_BTN_RF_SET                  2005
#define IDC_BTN_RF_GET                  2006
#define IDC_BTN_RF_PROBE                2007

// Log area
#define IDC_EDIT_LOG                    3001
#define IDC_BTN_CLEAR_LOG               3002

#endif // RESOURCE_H
