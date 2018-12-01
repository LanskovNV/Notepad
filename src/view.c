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
	return DefWindowProcW(hwnd, msg, wp, lp);
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

void CheckMode(HWND hwnd, int iSelection, HMENU hMenu, WPARAM wParam)
{
	CheckMenuItem(hMenu, iSelection, MF_UNCHECKED);
	iSelection = LOWORD(wParam);
	CheckMenuItem(hMenu, iSelection, MF_CHECKED);
	InvalidateRect(hwnd, NULL, TRUE);
}

int ResizeMsg(HWND hwnd, LPARAM lParam, RECT rect, MYTEXT *text, int *iMaxWidth, int *cxClient, int *cyClient, int *curNumLines, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos, int cxChar, int cyChar)
{
	int tmp = 0;
	int width = rect.right / cxChar;

	if (text->mode == classic)
		*iMaxWidth = text->maxWidth;
	else
		*iMaxWidth = *cxClient;

	*cxClient = LOWORD(lParam);
	*cyClient = HIWORD(lParam);
	if (text->mode == width)
	{
		GetClientRect(hwnd, &rect);
		BuildWidthStrings(text, rect.right / cxChar);
		*curNumLines = text->numWidthLines;
	}
	else
		*curNumLines = text->numLines;
	tmp = *curNumLines + 2 - *cyClient / cyChar;
	*iVscrollMax = max(tmp, 0);
	*iVscrollPos = min(*iVscrollPos, *iVscrollMax);
	SetScrollRange(hwnd, SB_VERT, 0, *iVscrollMax, FALSE);
	SetScrollPos(hwnd, SB_VERT, *iVscrollPos, TRUE);

	if (text->mode != width)
	{
		tmp = 2 + (*iMaxWidth - *cxClient) / cxChar;
		*iHscrollMax = max(0, tmp);
		*iHscrollPos = min(*iHscrollPos, *iHscrollMax);
		SetScrollRange(hwnd, SB_HORZ, 0, *iHscrollMax, FALSE);
		SetScrollPos(hwnd, SB_HORZ, *iHscrollPos, TRUE);
	}
	return 0;
}