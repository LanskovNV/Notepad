#include <windows.h>
#include "view.h"

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (wp)
		{
		case 1:
			DestroyWindow(hwnd);
			break;
		case 2:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	default:
		return DefWindowProcW(hwnd, msg, wp, lp);
	}
}

void RegisterDialogClass(HINSTANCE hInst)
{
	WNDCLASSW dialog = { 0 };

	dialog.hbrBackground = (HBRUSH)COLOR_WINDOW;
	dialog.hCursor = LoadCursor(NULL, IDC_ARROW);  
	dialog.hInstance = hInst;
	dialog.lpszClassName = L"Dialog";
	dialog.lpfnWndProc = DialogProcedure;

	RegisterClassW(&dialog);
}

void DisplayDialog(HWND hwnd)
{
	HWND hDlg = CreateWindowW(L"Dialog", L"Are you really want to exit?", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 750, 450, 300, 120, hwnd, NULL, NULL, NULL);

	CreateWindowW(L"Button", L"no", WS_VISIBLE | WS_CHILD, 160, 20, 100, 40, hDlg, (HMENU)1, NULL, NULL); 
	CreateWindowW(L"Button", L"yes", WS_VISIBLE | WS_CHILD, 20, 20, 100, 40, hDlg, (HMENU)2, NULL, NULL);
}