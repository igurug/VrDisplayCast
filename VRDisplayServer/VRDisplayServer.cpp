// VRDisplayServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "VRDisplayServer.h"
#include <shellapi.h>
#include <Ws2tcpip.h>
#include <WinSock2.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <Windows.h>
#include <iostream>
#include <thread>
#include "vrserver_func.h"



#define MAX_LOADSTRING 100
#define VR_ADD_IP 777
#define VR_START_SERVER 111
#define VR_STOP_SERVER  222
#define VR_RES_ORIGINAL 333
#define VR_RES_1280		444
#define VR_RES_1024		555
#define VR_RES_800		666
#define VR_EXIT			999

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HMENU hMenu;
NOTIFYICONDATA noti;
int desktopResolution = 0;
bool exitFlag = false;


// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    IpDiagCallback(HWND, UINT, WPARAM, LPARAM);
bool loadIp(WCHAR* buf);
bool saveIp(WCHAR* buf);



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
    LoadStringW(hInstance, IDC_VRDISPLAYSERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VRDISPLAYSERVER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		if (exitFlag)
			break;
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VRDISPLAYSERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VRDISPLAYSERVER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	

   HMENU hRes = CreatePopupMenu();
   
   switch (resolution)
   {
   case VR_RES_ORIGINAL:
	   AppendMenu(hRes, MF_STRING|MF_CHECKED, VR_RES_ORIGINAL, L"Original (slowest)");
	   AppendMenu(hRes, MF_STRING, VR_RES_1280, L"1280x1024");
	   AppendMenu(hRes, MF_STRING, VR_RES_1024, L"1024x768");
	   AppendMenu(hRes, MF_STRING, VR_RES_800, L"800x600 (fastest)");
	   break;
   case VR_RES_1280:
	   AppendMenu(hRes, MF_STRING, VR_RES_ORIGINAL, L"Original (slowest)");
	   AppendMenu(hRes, MF_STRING | MF_CHECKED, VR_RES_1280, L"1280x1024");
	   AppendMenu(hRes, MF_STRING, VR_RES_1024, L"1024x768");
	   AppendMenu(hRes, MF_STRING, VR_RES_800, L"800x600 (fastest)");
	   break;

   case VR_RES_1024:
	   AppendMenu(hRes, MF_STRING, VR_RES_ORIGINAL, L"Original (slowest)");
	   AppendMenu(hRes, MF_STRING, VR_RES_1280, L"1280x1024");
	   AppendMenu(hRes, MF_STRING | MF_CHECKED, VR_RES_1024, L"1024x768");
	   AppendMenu(hRes, MF_STRING, VR_RES_800, L"800x600 (fastest)");
	   break;

   case VR_RES_800:
	   AppendMenu(hRes, MF_STRING, VR_RES_ORIGINAL, L"Original (slowest)");
	   AppendMenu(hRes, MF_STRING, VR_RES_1280, L"1280x1024");
	   AppendMenu(hRes, MF_STRING, VR_RES_1024, L"1024x768");
	   AppendMenu(hRes, MF_STRING | MF_CHECKED, VR_RES_800, L"800x600 (fastest)");
	   break;

   }
   
   
   hMenu = CreatePopupMenu();
   WCHAR buf[100];
   if (loadIp(buf))
   {
	   char c[100];
	   sprintf_s(c, "%ws", buf);
	   strcpy_s(ipaddr, c);
	   AppendMenu(hMenu, MF_STRING, VR_ADD_IP, buf);
   }
   else
	   AppendMenu(hMenu, MF_STRING, VR_ADD_IP, L"Add IP address");
			   AppendMenu(hMenu, MF_STRING, VR_START_SERVER, L"Start Server");
    	       AppendMenu(hMenu, MF_STRING, VR_STOP_SERVER, L"Stop Server");
    	       AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRes, L"Resulotuin");
    	       AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    	       AppendMenu(hMenu, MF_STRING, 000, L"Help");
			   AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			   AppendMenu(hMenu, MF_STRING, VR_EXIT, L"Exit");
			   

   noti;
   noti.cbSize = sizeof(NOTIFYICONDATA);
   noti.hWnd = hWnd;
   noti.uID = 1110;
   noti.uFlags = NIF_ICON | NIF_MESSAGE;
   noti.uCallbackMessage = 1111;
   noti.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
   noti.uVersion = NOTIFYICON_VERSION;
   

   Shell_NotifyIcon(NIM_ADD, &noti);
  // ShowWindow(hWnd, nCmdShow);
  // UpdateWindow(hWnd);

   return TRUE;
}


void ShowContextMenu(HWND hWnd)
{
	// Get current mouse position.
	POINT curPoint;
	GetCursorPos(&curPoint);

	// should SetForegroundWindow according
	// to original poster so the popup shows on top
	SetForegroundWindow(hWnd);

	// TrackPopupMenu blocks the app until TrackPopupMenu returns
	UINT clicked = TrackPopupMenu(
		hMenu,
		0,
		curPoint.x,
		curPoint.y,
		0,
		hWnd,
		NULL
	);
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

thread* t1;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	//OutputDebugString(L"Main cycle...\n");
    switch (message)
    {
    case WM_COMMAND:
        {
		/*WCHAR buf[1000];
		_itow_s(message, buf, 10);
		OutputDebugString(buf);
		OutputDebugString(L"  ");
		_itow_s(lParam, buf, 10);
		OutputDebugString(buf);
		OutputDebugString(L"  ");
		_itow_s(wParam, buf, 10);
		OutputDebugString(buf);
		OutputDebugString(L"\n");*/
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case VR_ADD_IP:
				
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, IpDiagCallback);
				
				break;
			case VR_START_SERVER:
				//MessageBox(hWnd, L"Hello", L"Starting server...", MSGF_MESSAGEBOX);
				//printf("Starting server...");
				OutputDebugString(L"Starting server...\n");
				start_server();
				//run = true;
				//t1 = new thread(server_process);
				
				//t1->join();
				OutputDebugString(L"Server RUNNING...\n");
				break;
			case VR_STOP_SERVER:
				stop_server();
				break;

			case VR_RES_ORIGINAL:
				setResolution(VR_RES_ORIGINAL);
				ModifyMenu(hMenu, VR_RES_ORIGINAL, MF_BYCOMMAND | MF_CHECKED, VR_RES_ORIGINAL, L"Original (slowest)");
				ModifyMenu(hMenu, VR_RES_1280, MF_BYCOMMAND, VR_RES_1280, L"1280x1024");
				ModifyMenu(hMenu, VR_RES_1024, MF_BYCOMMAND, VR_RES_1024, L"1024x768");
				ModifyMenu(hMenu, VR_RES_800, MF_BYCOMMAND, VR_RES_800, L"800x600 (fastest)");
				break;
			case VR_RES_1280:
				setResolution(VR_RES_1280);
				ModifyMenu(hMenu, VR_RES_ORIGINAL, MF_BYCOMMAND, VR_RES_ORIGINAL, L"Original (slowest)");
				ModifyMenu(hMenu, VR_RES_1280, MF_BYCOMMAND | MF_CHECKED, VR_RES_1280, L"1280x1024");
				ModifyMenu(hMenu, VR_RES_1024, MF_BYCOMMAND, VR_RES_1024, L"1024x768");
				ModifyMenu(hMenu, VR_RES_800, MF_BYCOMMAND, VR_RES_800, L"800x600 (fastest)");
				break;
			case VR_RES_1024:
				setResolution(VR_RES_1024);
				ModifyMenu(hMenu, VR_RES_ORIGINAL, MF_BYCOMMAND, VR_RES_ORIGINAL, L"Original (slowest)");
				ModifyMenu(hMenu, VR_RES_1280, MF_BYCOMMAND, VR_RES_1280, L"1280x1024");
				ModifyMenu(hMenu, VR_RES_1024, MF_BYCOMMAND | MF_CHECKED, VR_RES_1024, L"1024x768");
				ModifyMenu(hMenu, VR_RES_800, MF_BYCOMMAND, VR_RES_800, L"800x600 (fastest)");
				break;
			case VR_RES_800:
				setResolution(VR_RES_800);
				ModifyMenu(hMenu, VR_RES_ORIGINAL, MF_BYCOMMAND, VR_RES_ORIGINAL, L"Original (slowest)");
				ModifyMenu(hMenu, VR_RES_1280, MF_BYCOMMAND, VR_RES_1280, L"1280x1024");
				ModifyMenu(hMenu, VR_RES_1024, MF_BYCOMMAND, VR_RES_1024, L"1024x768");
				ModifyMenu(hMenu, VR_RES_800, MF_BYCOMMAND | MF_CHECKED, VR_RES_800, L"800x600 (fastest)");
				break;
			case VR_EXIT:
				exitFlag = true;
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
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_LBUTTONUP:

		break;

	case 1111:
		
		switch (lParam)
		{
		case WM_RBUTTONUP:
			//MessageBox(hWnd, L"Hello", L"Hello", MSGF_MESSAGEBOX);
			ShowContextMenu(hWnd);
			break;

		
		}

		//MessageBox(hWnd, L"Hello", L"Hello", MSGF_MESSAGEBOX);
		
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// Message handler for about box.
INT_PTR CALLBACK IpDiagCallback(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			WCHAR buf[100];
			GetDlgItemText(hDlg, IDC_IPADDRESS1, buf, 80);
			
			char c[100];
			sprintf_s(c, "%ws", buf);
			strcpy_s(ipaddr, c);
			ModifyMenu(hMenu, VR_ADD_IP, MF_BYCOMMAND, MF_STRING, buf);
			//MessageBox(0, buf, L"result", MSGF_MESSAGEBOX);
			EndDialog(hDlg, LOWORD(wParam));
			saveIp(buf);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool saveIp(WCHAR* buf)
{
	HKEY hKey;
	LPCTSTR sk = TEXT("Software\\VRDisplayServer");

	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);

	if (openRes == ERROR_SUCCESS) {
		OutputDebugString(L"Success opening key.");
	}
	else {
		OutputDebugString(L"Error opening key.");

		LONG openRes = RegCreateKey(HKEY_CURRENT_USER, sk, &hKey);
		if (openRes != ERROR_SUCCESS) {
			OutputDebugString(L"Error creating key.");
			return false;
		}
	}

	LPCTSTR value = TEXT("IpAddress");


	LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)buf, wcslen(buf) * 2 + 1);

	if (setRes == ERROR_SUCCESS) {
		OutputDebugString(L"Success writing to Registry.");
	}
	else {
		OutputDebugString(L"Error writing to Registry.");
		return false;
	}

	LONG closeOut = RegCloseKey(hKey);

	if (closeOut == ERROR_SUCCESS) {
		OutputDebugString(L"Success closing key.");
	}
	else {
		OutputDebugString(L"Error closing key.");
		return false;
	}
	return true;
}


bool loadIp(WCHAR* buf)
{
	HKEY hKey;
	LPCTSTR sk = TEXT("Software\\VRDisplayServer");


	LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_QUERY_VALUE, &hKey);

	if (openRes == ERROR_SUCCESS) {
		OutputDebugString(L"Success opening key.");
	}
	else {
		OutputDebugString(L"Error opening key.");

		LONG openRes = RegCreateKey(HKEY_CURRENT_USER, sk, &hKey);
		if (openRes != ERROR_SUCCESS) {
			OutputDebugString(L"Error creating key.");
			return false;
		}
	}
	LPCTSTR value = TEXT("IpAddress");

	DWORD size;
	LONG setRes = RegGetValue(HKEY_CURRENT_USER, sk, value, RRF_RT_REG_SZ, NULL, (PVOID)buf, &size);

	if (setRes == ERROR_SUCCESS) {
		OutputDebugString(L"Success reading from Registry.");
	}
	else {
		OutputDebugString(L"Error reading from Registry.");
		return false;
	}

	
	return true;
}