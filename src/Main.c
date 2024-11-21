#include "Array.h"
#include "Memory.h"

#define SEARCH_BTN_ID 101
#define CODE_LABEL_ID 102
#define LISTBOX_ID 103

char pattern[] = "00 00 08 00 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 00 00 00 00 08 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

HFONT defaultFont;
HWND hSearchBtn, hCodeLabel, hListBox;

DWORD WINAPI FindCodes(HANDLE* hProcess) {
	EnableWindow(hSearchBtn, FALSE);
	SetWindowText(hSearchBtn, "Searching...");
	
	Array foundAddrArr;
	InitArray(&foundAddrArr);
	
	FindPattern(hProcess, pattern, &foundAddrArr);

	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	
	for (int i = 0; i < foundAddrArr.size; i++) {
		UINT64 address = foundAddrArr.data[i];
		int value = ReadInt(hProcess, address + 0x372);
		
		if (0 < value && value < 10000) {
			char buffer[16];
			wsprintfA(buffer, "%04d", value);
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)buffer);
		}
	}

	EnableWindow(hSearchBtn, TRUE);
	SetWindowText(hSearchBtn, "Search");

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HANDLE hProcess;
	static HINSTANCE hInstance;
	
	switch (uMsg) {
	case WM_CREATE:
		hInstance = (HINSTANCE)GetModuleHandle(NULL);

		defaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		hSearchBtn = CreateWindowEx(0, "BUTTON", "Search", WS_CHILD | WS_VISIBLE, 0, 0, 100, 25, hWnd, (HMENU)SEARCH_BTN_ID, hInstance, NULL);
		hCodeLabel = CreateWindowEx(0, "STATIC", "Codes Found", WS_CHILD | WS_VISIBLE | SS_CENTER, 15, 15, 100, 15, hWnd, (HMENU)CODE_LABEL_ID, hInstance, NULL);
		hListBox = CreateWindowEx(0, "LISTBOX", "", WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL, 0, 0, 100, 100, hWnd, (HMENU)LISTBOX_ID, hInstance, NULL);

		SendMessage(hSearchBtn, WM_SETFONT, (WPARAM)defaultFont, TRUE);
		SendMessage(hCodeLabel, WM_SETFONT, (WPARAM)defaultFont, TRUE);
		SendMessage(hListBox, WM_SETFONT, (WPARAM)defaultFont, TRUE);

		return 0;

	case WM_DESTROY:
		DeleteObject(defaultFont);
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		SetWindowPos(hSearchBtn, HWND_TOP, (LOWORD(lParam) - 100) >> 1, HIWORD(lParam) - 40, 0, 0, SWP_NOSIZE);
		SetWindowPos(hCodeLabel, HWND_TOP, (LOWORD(lParam) - 100) >> 1, 15, 0, 0, SWP_NOSIZE);
		SetWindowPos(hListBox, HWND_TOP, 15, 45, LOWORD(lParam) - 30, HIWORD(lParam) - 100, 0);
		return 0;

	case WM_COMMAND:
		if (LOWORD(wParam) == SEARCH_BTN_ID && HIWORD(wParam) == BN_CLICKED) {						
			if (hProcess == NULL || (hProcess = GetProcessHandle("PAYDAY3Client-Win64-Shipping.exe")) == NULL) {
				MessageBox(hWnd, "The game is not open!", "Code Finder", MB_OK | MB_ICONINFORMATION);
				return 0;
			}

			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FindCodes, hProcess, 0, NULL);
		}

		return 0;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hDc = BeginPaint(hWnd, &ps);
		FillRect(hDc, &ps.rcPaint, (HBRUSH)COLOR_WINDOW);
		EndPaint(hWnd, &ps);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg = { };
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = "rootWindow";

	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(0, "rootWindow", "Code Finder", WS_SYSMENU | WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, 250, 243, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
