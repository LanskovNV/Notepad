#include <windows.h>
#include "view.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static char szAppName[] = "Notepad";
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	HMENU hMenu = CreateMenu();
	HMENU hMenuPopup = CreateMenu();
	AppendMenu(hMenuPopup, MF_STRING, IDM_OPEN, "&Open file...");
	AppendMenu(hMenuPopup, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenuPopup, MF_STRING, IDM_EXIT, "E&xit");
	AppendMenu(hMenu, MF_POPUP, (UINT)hMenuPopup, "&File");
	hMenuPopup = CreateMenu();
	AppendMenu(hMenuPopup, MF_STRING | MF_CHECKED, IDM_CLASSIC, "&classic");
	AppendMenu(hMenuPopup, MF_STRING, IDM_WIDTH, "&width");
	AppendMenu(hMenu, MF_POPUP, (UINT)hMenuPopup, "&Mode");

	if (!RegisterClassEx(&wndclass))
		return -1;
	
	RegisterDialogClass(hInstance);

	hwnd = CreateWindow(
		szAppName,
		"MyNotepad",
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, hMenu, hInstance, NULL
	);
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static int cxChar, cyChar, cxClient, cyClient, iSelection = IDM_CLASSIC, xCaret, yCaret;
	static long iVscrollPos, iVscrollMax, iHscrollPos, iHscrollMax, iMaxWidth;
	static MYTEXT text;

	if (text.numLines == 0)
		LoadText(&text, FILE_NAME);

	switch (iMsg)
	{
	case WM_CREATE:
		return CreateMsg(hwnd, &cxChar, &cyChar);
	case WM_SIZE:
		return ResizeMsg(hwnd, lParam, &text, &iMaxWidth, &cxClient, &cyClient, &iVscrollMax, &iVscrollPos, &iHscrollMax, &iHscrollPos, cxChar, cyChar);
	case WM_SETFOCUS:

		CreateCaret(hwnd, NULL, cxChar, cyChar);
		SetCaretPos(xCaret * cxChar, yCaret * cyChar);
		ShowCaret(hwnd);
		return 0;

	case WM_KILLFOCUS:

		HideCaret(hwnd);
		DestroyCaret();
		return 0;

	case WM_KEYDOWN:
		return KeydownMsg(hwnd, wParam, &text, &xCaret, &yCaret, cxChar, cyChar, cxClient, cyClient);
	case WM_VSCROLL:
		return VscrollMsg(hwnd, wParam, &iVscrollPos, iVscrollMax, cyClient, cyChar);
	case WM_HSCROLL:
		return HscrollMsg(text.mode, wParam, hwnd, &iHscrollPos, iHscrollMax, cxChar);
	case WM_PAINT:
		return PaintMsg(hwnd, &text, iVscrollPos, iHscrollPos, cxChar, cyChar);
	case WM_COMMAND:
		return CommandMsg(hwnd,wParam,lParam, &text, &iSelection, cxChar, cyChar, &iMaxWidth, &cxClient, &cyClient, &iVscrollMax, &iVscrollPos, &iHscrollMax, &iHscrollPos);
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		DisplayDialog(hwnd);
		return 0;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}