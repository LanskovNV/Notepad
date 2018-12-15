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

void CheckMode(HWND hwnd, int *iSelection, HMENU hMenu, WPARAM wParam)
{
	CheckMenuItem(hMenu, *iSelection, MF_UNCHECKED);
	*iSelection = LOWORD(wParam);
	CheckMenuItem(hMenu, *iSelection, MF_CHECKED);
	InvalidateRect(hwnd, NULL, TRUE);
}

int ResizeMsg(HWND hwnd, LPARAM lParam, MYTEXT *text, int *iMaxWidth, int *cxClient, int *cyClient, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos, int cxChar, int cyChar)
{
	int tmp = 0;
	int nwidth, curNumLines;
	RECT rect;

	if (text->mode == classic)
		*iMaxWidth = text->maxWidth;
	else
		*iMaxWidth = *cxClient;

	*cxClient = LOWORD(lParam);
	*cyClient = HIWORD(lParam);
	GetClientRect(hwnd, &rect);
	nwidth = rect.right / cxChar - 1;

	if (text->mode == width && (text->widthStrings == NULL || text->curWidth != nwidth))
	{
		BuildWidthStrings(text, nwidth);
		curNumLines = text->numWidthLines;
	}
	else
		curNumLines = text->numLines;
	tmp = curNumLines + 2 - *cyClient / cyChar;
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

int CreateMsg(HWND hwnd, int *cxChar, int *cyChar)
{
	HFONT hfnt;
	TEXTMETRIC tm;
	HDC hdc;

	hfnt = GetStockObject(SYSTEM_FIXED_FONT);
	hdc = GetDC(hwnd);
	SelectObject(hdc, hfnt);
	GetTextMetrics(hdc, &tm);
	* cxChar = tm.tmMaxCharWidth;
	* cyChar = tm.tmHeight + tm.tmExternalLeading;
	ReleaseDC(hwnd, hdc);
	return 0;
}

int PaintMsg(HWND hwnd, MYTEXT *text, int iVscrollPos, int iHscrollPos, int cxChar, int cyChar)
{
	PAINTSTRUCT ps;
	LPSTR *curStrings = NULL;
	int curLen = 0, iPaintBeg, iPaintEnd, x, y, i;
	HDC hdc;

	curStrings = SelectStrings(*text);
	curLen = SelectNOfLines(*text);
	hdc = BeginPaint(hwnd, &ps);
	SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
	iPaintBeg = max(0, iVscrollPos + ps.rcPaint.top / cyChar - 1);
	iPaintEnd = min((int)curLen, iVscrollPos + ps.rcPaint.bottom / cyChar);

	for (i = iPaintBeg; i < iPaintEnd; i++)
	{
		x = cxChar * (-1 * iHscrollPos);
		y = cyChar * (i - iVscrollPos);
		TextOut(
			hdc, x, y,
			curStrings[i],
			strlen(curStrings[i])
		);
		SetTextAlign(hdc, TA_LEFT | TA_TOP);
	}
	EndPaint(hwnd, &ps);
	return 0;
}

int KeydownMsg(HWND hwnd, WPARAM wParam, MYTEXT *text, int *xCaret, int *yCaret, int cxChar, int cyChar, int cxClient, int cyClient)
{
	int cxBuffer = max(1, cxClient / cxChar);
	int cyBuffer = max(1, cyClient / cyChar);

	switch (wParam)
	{
	case VK_PRIOR:
		SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
		break;
	case VK_NEXT:
		SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
		break;
	case VK_UP:
		UpdateClassPos(hwnd, text, up, xCaret, yCaret, cxBuffer, cyBuffer);
		break;
	case VK_DOWN:
		UpdateClassPos(hwnd, text, down, xCaret, yCaret, cxBuffer, cyBuffer);
		break;
	case VK_LEFT:
		UpdateClassPos(hwnd, text, left, xCaret, yCaret, cxBuffer, cyBuffer);
		break;
	case VK_RIGHT:
		UpdateClassPos(hwnd, text, right, xCaret, yCaret, cxBuffer, cyBuffer);
		break;
	}

	SetCaretPos(*xCaret * cxChar, *yCaret * cyChar);
	return 0;
}

int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, MYTEXT *text, int *iSelection, int cxChar, int cyChar, int *iMaxWidth, int *cxClient, int *cyClient, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos)
{
	HMENU hMenu = GetMenu(hwnd);
	int isClassic = 1;

	switch (LOWORD(wParam))
	{
	case IDM_OPEN:
		OpenFileFunc(hwnd, text, *cxClient);
		return 0;
	case IDM_EXIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0L);
		return 0;
	case IDM_WIDTH: // assumes that IDM_WHITE
		text->mode = width;
		isClassic = ResizeMsg(hwnd, lParam, text, iMaxWidth, cxClient, cyClient, iVscrollMax, iVscrollPos, iHscrollMax, iHscrollPos, cxChar, cyChar);
	case IDM_CLASSIC: // Note: Logic below
		if (isClassic)
		{
			text->mode = classic;
			isClassic = 1;
		}
		CheckMode(hwnd, iSelection, hMenu, wParam);
		return 0;
	}
	return 0;
}

int VscrollMsg(HWND hwnd, WPARAM wParam, int *iVscrollPos, int iVscrollMax, int cyClient, int cyChar)
{
	int iVscrollInc = 0;

	switch (LOWORD(wParam))
	{
	case SB_TOP:
		iVscrollInc = -*iVscrollPos;
		break;
	case SB_BOTTOM:
		iVscrollInc = iVscrollMax - *iVscrollPos;
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
		iVscrollInc = HIWORD(wParam) - *iVscrollPos;
		break;
	default:
		iVscrollInc = 0;
	}
	iVscrollInc = max(
		-*iVscrollPos,
		min(iVscrollInc, iVscrollMax - *iVscrollPos)
	);
	if (iVscrollInc != 0)
	{
		*iVscrollPos += iVscrollInc;
		ScrollWindow(hwnd, 0, -cyChar * iVscrollInc, NULL, NULL);
		SetScrollPos(hwnd, SB_VERT, *iVscrollPos, TRUE);
		UpdateWindow(hwnd);
	}
	return 0;
}

int HscrollMsg(mode_t mode, WPARAM wParam, HWND hwnd, int *iHscrollPos, int iHscrollMax, int cxChar)
{
	int iHscrollInc = 0;

	if (mode != width)
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
			iHscrollInc = HIWORD(wParam) - *iHscrollPos;
			break;
		default:
			iHscrollInc = 0;
		}
		iHscrollInc = max(
			-*iHscrollPos,
			min(iHscrollInc, iHscrollMax - *iHscrollPos)
		);
		if (iHscrollInc != 0)
		{
			*iHscrollPos += iHscrollInc;
			ScrollWindow(hwnd, -cxChar * iHscrollInc, 0, NULL, NULL);
			SetScrollPos(hwnd, SB_HORZ, *iHscrollPos, TRUE);
		}
	}
	return 0;
}
