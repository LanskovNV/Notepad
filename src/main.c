#include <windows.h>
#include "model.h"
#include "view.h"

#define IDM_OPEN 1
#define IDM_EXIT 2
#define IDM_CLASSIC 3
#define IDM_WIDTH 4

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
	static int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth,
		iVscrollPos, iVscrollMax, iHscrollPos, iHscrollMax, tmp;
	HDC hdc;
	HMENU hMenu;
	int i, x, y, iPaintBeg, iPaintEnd, iVscrollInc, iHscrollInc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	static int iSelection = IDM_CLASSIC;
	static MYTEXT text;
	LPSTR *curStrings = NULL;
    RECT rect;
	DWORD curLen = 0, curNumLines = 0;
	GetClientRect(hwnd, &rect);
	int isClassic = 1;

	if (text.numLines == 0)
		LoadText(&text, FILE_NAME);

	switch (iMsg)
	{
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);		
		return 0;
	case WM_SIZE:

		if (text.mode == classic)
			iMaxWidth = text.maxWidth;
		else
			iMaxWidth = cxClient;

		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		if (text.mode == width)
		{
			GetClientRect(hwnd, &rect);
			BuildWidthStrings(&text, rect.right, cxChar);
			curNumLines = text.numWidthLines;
		}
		else
		{
			curNumLines = text.numLines;
		}
		tmp = curNumLines + 2 - cyClient / cyChar;
		iVscrollMax = max(tmp, 0);
		iVscrollPos = min(iVscrollPos, iVscrollMax);
		SetScrollRange(hwnd, SB_VERT, 0, iVscrollMax, FALSE);
		SetScrollPos(hwnd, SB_VERT, iVscrollPos, TRUE);

		if (text.mode != width)
		{
			tmp = 2 + (iMaxWidth - cxClient) / cxChar;
			iHscrollMax = max(0, tmp);
			iHscrollPos = min(iHscrollPos, iHscrollMax);
			SetScrollRange(hwnd, SB_HORZ, 0, iHscrollMax, FALSE);
			SetScrollPos(hwnd, SB_HORZ, iHscrollPos, TRUE);
		}
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_PRIOR:
			SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
			break;
		case VK_NEXT:
			SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
			break;
		case VK_UP:
			SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
			break;
		case VK_DOWN:
			SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
			break;
		case VK_LEFT:
			SendMessage(hwnd, WM_HSCROLL, SB_PAGEUP, 0L);
			break;
		case VK_RIGHT:
			SendMessage(hwnd, WM_HSCROLL, SB_PAGEDOWN, 0L);
			break;
		}
		return 0;
	case WM_VSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			iVscrollInc = -iVscrollPos;
			break;
		case SB_BOTTOM:
			iVscrollInc = iVscrollMax - iVscrollPos;
			break;
		case SB_LINEUP:
			iVscrollInc = -1;
			break;
		case SB_LINEDOWN:
			iVscrollInc = 1;
			break;
		case SB_PAGEUP:
			iVscrollInc = min(-1, -cyClient / cyChar);
			break;
		case SB_PAGEDOWN:
			iVscrollInc = max(1, cyClient / cyChar);
				break;
		case SB_THUMBTRACK:
			iVscrollInc = HIWORD(wParam) - iVscrollPos;
			break;
		default:
			iVscrollInc = 0;
		}
		iVscrollInc = max(
			-iVscrollPos,
			min(iVscrollInc, iVscrollMax - iVscrollPos)
		);
		if (iVscrollInc != 0)
		{
			iVscrollPos += iVscrollInc;
			ScrollWindow(hwnd, 0, -cyChar * iVscrollInc, NULL, NULL);
			SetScrollPos(hwnd, SB_VERT, iVscrollPos, TRUE);
			UpdateWindow(hwnd);
		}
		return 0;
	case WM_HSCROLL:
		if (text.mode != width)
		{
			switch (LOWORD(wParam))
			{
			case SB_LINEUP:
				iHscrollInc = -1;
				break;
			case SB_LINEDOWN:
				iHscrollInc = 1;
				break;
			case SB_PAGEUP:
				iHscrollInc = -8;
				break;
			case SB_PAGEDOWN:
				iHscrollInc = 8;
				break;
			case SB_THUMBPOSITION:
				iHscrollInc = HIWORD(wParam) - iHscrollPos;
				break;
			default:
				iHscrollInc = 0;
			}
			iHscrollInc = max(
				-iHscrollPos,
				min(iHscrollInc, iHscrollMax - iHscrollPos)
			);
			if (iHscrollInc != 0)
			{
				iHscrollPos += iHscrollInc;
				ScrollWindow(hwnd, -cxChar * iHscrollInc, 0, NULL, NULL);
				SetScrollPos(hwnd, SB_HORZ, iHscrollPos, TRUE);
			}
		}
		return 0;
	case WM_PAINT:
		curStrings = SelectStrings(text);
		curLen = SelectNOfLines(text);
		hdc = BeginPaint(hwnd, &ps);
		iPaintBeg = max(0, iVscrollPos + ps.rcPaint.top / cyChar - 1);
		iPaintEnd = min((int)curLen, iVscrollPos + ps.rcPaint.bottom / cyChar);
		
		for (i = iPaintBeg; i < iPaintEnd; i++)
		{
			x = cxChar * (1 - iHscrollPos);
			y = cyChar * (1 - iVscrollPos + i);
			TextOut(
				hdc, x, y,
				curStrings[i],
				strlen(curStrings[i])
			);
			SetTextAlign(hdc, TA_LEFT | TA_TOP);
		}
		EndPaint(hwnd, &ps);
		return 0;
	case WM_COMMAND:
		hMenu = GetMenu(hwnd);

		switch (LOWORD(wParam))
		{
		case IDM_OPEN:
			OpenFileFunc(hwnd, &text, rect.right);
			return 0;
		case IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0L);
			return 0;
		case IDM_WIDTH: // assumes that IDM_WHITE
			text.mode = width;
			isClassic = BuildWidthStrings(&text, cxClient, cxChar);
		case IDM_CLASSIC: // Note: Logic below
			if (isClassic)
			{
				text.mode = classic;
				isClassic = 1;
			}
			CheckMenuItem(hMenu, iSelection, MF_UNCHECKED);
			iSelection = LOWORD(wParam);
			CheckMenuItem(hMenu, iSelection, MF_CHECKED);
			InvalidateRect(hwnd, NULL, TRUE);
			SendMessage(hwnd, WM_SIZE, 0, 0L);
			return 0;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		DisplayDialog(hwnd);
		return 0;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}