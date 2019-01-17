#include <windows.h>
#include <stdio.h>
#include "text.h"
#include "view.h"

/***************************************
	Processing movements (arrows etc.) */

/*
static int IsLastSpaces(LPSTR buf, int bufLen, int curLen)
{
	int ans = curLen == 1 ? 0 : 1;
	int i;

	for (i = 0; i < bufLen; i++)
		if (!IsSpace(*buf))
		{
			ans = 0;
			break;
		}
	return ans;
}
*/

static void MoveLeft(HWND hwnd, view_t *view, text_t *text)
{
	LPSTR *buffer = SelectStrings(*text);
	int nOfLines = SelectNOfLines(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	if (text->pos.x >= 0)
	{
		text->pos.x--;
		if (text->pos.x < cxBuffer)
			view->caret.x++;
		else
		{
			view->iHscrollPos += 1;
			ScrollWindow(hwnd, -view->charSize.x, 0, NULL, NULL);
			SetScrollPos(hwnd, SB_HORZ, view->iHscrollPos, TRUE);
		}
	}
	else if (text->pos.y < nOfLines)
	{
		text->pos.x = 0;
		text->pos.y++;
		view->caret.x = 0;

		ScrollWindow(hwnd, view->iHscrollPos * view->charSize.x, 0, NULL, NULL);
		view->iHscrollPos = 0;
		SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

		if (text->pos.y < cyBuffer)
			view->caret.y++;
		else
		{
			view->iVscrollPos += 1;
			ScrollWindow(hwnd, 0, -view->charSize.y, NULL, NULL);
			SetScrollPos(hwnd, SB_VERT, view->iVscrollPos, TRUE);
		}
	}
}

static void MoveRight(HWND hwnd, view_t *view, text_t *text)
{
	LPSTR *buffer = SelectStrings(*text);
	int nOfLines = SelectNOfLines(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	if (text->pos.x < (int)strlen(buffer[text->pos.y]) - 2)
	{
		text->pos.x++;
		if (text->pos.x < cxBuffer)
			view->caret.x++;
		else
		{
			view->iHscrollPos += 1;
			ScrollWindow(hwnd, -view->charSize.x, 0, NULL, NULL);
			SetScrollPos(hwnd, SB_HORZ, view->iHscrollPos, TRUE);
		}
	}
	else if (text->pos.y < nOfLines)
	{
		text->pos.x = 0;
		text->pos.y++;
		view->caret.x = 0;

		ScrollWindow(hwnd, view->iHscrollPos * view->charSize.x, 0, NULL, NULL);
		view->iHscrollPos = 0;
		SetScrollPos(hwnd, SB_HORZ, 0, TRUE);

		if (text->pos.y < cyBuffer)
			view->caret.y++;
		else
		{
			view->iVscrollPos += 1;
			ScrollWindow(hwnd, 0, -view->charSize.y, NULL, NULL);
			SetScrollPos(hwnd, SB_VERT, view->iVscrollPos, TRUE);
		}
	}
}

static int UpdatePos(HWND hwnd, text_t *text, view_t *view, case_t c)
{
	LPSTR *buffer = SelectStrings(*text);
	pos_t curPos = text->pos;
	int numClStrings = SelectNOfLines(*text);
	int lastLineLen = strlen(buffer[numClStrings - 1]);
	int curStringLen = strlen(buffer[curPos.y]);

	switch (c)
	{
	case right:
	    MoveRight(hwnd, view, text);
		break;
	case left:
		MoveLeft(hwnd, view, text);
		break;
	case up:
		if (curPos.y > 0)
		{
			curPos.y -= 1;
			curStringLen = strlen(buffer[curPos.y]);
			if (curPos.x > curStringLen)
				curPos.x = curStringLen - 1;

			if (IsSpace(buffer[curPos.y][curPos.x]))
			{
				MoveLeft(hwnd, view, text);
			}
		}
		break;
	case down:
		if (curPos.y < numClStrings - 1)
		{
			curPos.y += 1;
			curStringLen = strlen(buffer[curPos.y]);
			if (curStringLen <= curPos.x)
				curPos.x = max(0, curStringLen - 1);

			if (IsSpace(buffer[curPos.y][curPos.x]))
			{
				if (IsLastSpaces(buffer[curPos.y] + curPos.x, curStringLen - curPos.x, curStringLen))
					MoveLeft(hwnd, view, text);
				else
					MoveRight(hwnd, view, text);
			}
		}
		break;
	default:
		printf("error in UpdatePos func: incorrect parameter c");
		return 1;
	}

	/*
	text->pos = curPos;
	if (chX)
		view->caret.x = text->pos.x;
	if (chY)
		view->caret.y = text->pos.y;
    */
	return 0;
}

/********************************
	Windows messages processing */

static void CheckMode(HWND hwnd, int *iSelection, HMENU hMenu, WPARAM wParam)
{
	CheckMenuItem(hMenu, *iSelection, MF_UNCHECKED);
	*iSelection = LOWORD(wParam);
	CheckMenuItem(hMenu, *iSelection, MF_CHECKED);
	InvalidateRect(hwnd, NULL, TRUE);
}

int KeydownMsg(HWND hwnd, WPARAM wParam, text_t *text, view_t *view)
{
	switch (wParam)
	{
	case VK_PRIOR:
		SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0L);
		break;
	case VK_NEXT:
		SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0L);
		break;
	case VK_UP:
		UpdatePos(hwnd, text, view, up);
		break;
	case VK_DOWN:
		UpdatePos(hwnd, text, view, down);
		break;
	case VK_LEFT:
		UpdatePos(hwnd, text, view, left);
		break;
	case VK_RIGHT:
		UpdatePos(hwnd, text, view, right);
		break;
	}

	SetCaretPos(view->caret.x * view->charSize.x, view->caret.y * view->charSize.y);
	return 0;
}

int VscrollMsg(HWND hwnd, WPARAM wParam, view_t *view)
{
	int iVscrollInc = 0;

	switch (LOWORD(wParam))
	{
	case SB_TOP:
		iVscrollInc = -view->iVscrollPos;
		break;
	case SB_BOTTOM:
		iVscrollInc = view->iVscrollMax - view->iVscrollPos;
		break;
	case SB_LINEUP:
		iVscrollInc = -1;
		break;
	case SB_LINEDOWN:
		iVscrollInc = 1;
		break;
	case SB_PAGEUP:
		iVscrollInc = min(-1, -view->client.y / view->charSize.y);
		break;
	case SB_PAGEDOWN:
		iVscrollInc = max(1, view->client.y / view->charSize.y); 
		break;
	case SB_THUMBTRACK:
		iVscrollInc = HIWORD(wParam) - view->iVscrollPos;
		break;
	default:
		iVscrollInc = 0;
	}
	iVscrollInc = max(
		-view->iVscrollPos,
		min(iVscrollInc, view->iVscrollMax - view->iVscrollPos)
	);
	if (iVscrollInc != 0)
	{
		view->iVscrollPos += iVscrollInc;
		ScrollWindow(hwnd, 0, -view->charSize.y * iVscrollInc, NULL, NULL);
		SetScrollPos(hwnd, SB_VERT, view->iVscrollPos, TRUE);
		UpdateWindow(hwnd);
	}
	return 0;
}

int HscrollMsg(mode_t mode, WPARAM wParam, HWND hwnd, view_t *view)
{
	int iHscrollInc = 0;

	if (mode != transfer)
	{
		switch (LOWORD(wParam))
		{
		case SB_RIGHT:
			iHscrollInc = view->iHscrollMax - view->iHscrollPos;
			break;
		case SB_LEFT:
			iHscrollInc = -view->iHscrollPos;
			break;
		case SB_LINEUP:
			iHscrollInc = -1;
			break;
		case SB_LINEDOWN:
			iHscrollInc = 1;
			break;
		case SB_THUMBTRACK:
			iHscrollInc = HIWORD(wParam) - view->iHscrollPos;
			break;
		default:
			iHscrollInc = 0;
		}
		iHscrollInc = max(
			-view->iHscrollPos,
			min(iHscrollInc, view->iHscrollMax - view->iHscrollPos)
		);
		if (iHscrollInc != 0)
		{
			view->iHscrollPos += iHscrollInc;
			ScrollWindow(hwnd, -view->charSize.x * iHscrollInc, 0, NULL, NULL);
			SetScrollPos(hwnd, SB_HORZ, view->iHscrollPos, TRUE);
			UpdateWindow(hwnd);
		}
	}
	return 0;
}

int ResizeMsg(HWND hwnd, LPARAM lParam, text_t *text, view_t *view)
{
	int tmp = 0;
	int isClassic = 0;
	int nwidth, curnumClStrings;
	RECT rect;

	if (LOWORD(lParam) != 0)
		view->client.x = LOWORD(lParam);
	if (HIWORD(lParam) != 0)
		view->client.y = HIWORD(lParam);
	GetClientRect(hwnd, &rect);
	nwidth = rect.right / view->charSize.x - 1;

	if (text->mode == classic)
	{
		isClassic = 1;
		curnumClStrings = text->numClStrings;
		view->iMaxWidth = text->maxWidth;
	}
	else // text->mode == transfer
	{
		if (text->trStrings == NULL || text->curWidth != nwidth)
			BuildTrStrings(text, nwidth);
		curnumClStrings = text->numTrStrings;
		view->iMaxWidth = text->curWidth;
	}
	
	/* horizontal scroll */
	tmp = view->iMaxWidth - view->client.x / view->charSize.x;
	view->iHscrollMax = max(0, tmp);
	view->iHscrollPos = min(view->iHscrollPos, view->iHscrollMax);
	SetScrollRange(hwnd, SB_HORZ, 0, view->iHscrollMax, FALSE);
	SetScrollPos(hwnd, SB_HORZ, view->iHscrollPos, TRUE);

	/* vertical scroll */
	tmp = curnumClStrings - view->client.y / view->charSize.y;
	view->iVscrollMax = max(tmp, 0);
	view->iVscrollPos = min(view->iVscrollPos, view->iVscrollMax);
	SetScrollRange(hwnd, SB_VERT, 0, view->iVscrollMax, FALSE);
	SetScrollPos(hwnd, SB_VERT, view->iVscrollPos, TRUE);

	return isClassic;
}

int PaintMsg(HWND hwnd, text_t *text, view_t *view)
{
	PAINTSTRUCT ps;
	LPSTR *curStrings = NULL;
	int curLen = 0, iPaintBeg, iPaintEnd, x, y, i;
	HDC hdc;

	curStrings = SelectStrings(*text);
	curLen = SelectNOfLines(*text);
	hdc = BeginPaint(hwnd, &ps);
	SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
	iPaintBeg = max(0, view->iVscrollPos + ps.rcPaint.top / view->charSize.y - 1);
	iPaintEnd = min((int)curLen, view->iVscrollPos + ps.rcPaint.bottom / view->charSize.y);

	for (i = iPaintBeg; i < iPaintEnd; i++)
	{
		x = view->charSize.x * (-1 * view->iHscrollPos);
		y = view->charSize.y * (i - view->iVscrollPos);
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

int CreateMsg(HWND hwnd, view_t *view)
{
	HFONT hfnt;
	TEXTMETRIC tm;
	HDC hdc;

	hfnt = GetStockObject(SYSTEM_FIXED_FONT);
	hdc = GetDC(hwnd);
	SelectObject(hdc, hfnt);
	GetTextMetrics(hdc, &tm);
	view->charSize.x = tm.tmMaxCharWidth;
	view->charSize.y = tm.tmHeight + tm.tmExternalLeading;
	ReleaseDC(hwnd, hdc);

	return 0;
}

int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, text_t *text, view_t *view)
{
	HMENU hMenu = GetMenu(hwnd);
	int isClassic = 1;

	switch (LOWORD(wParam))
	{
	case IDM_OPEN:
		OpenFileFunc(hwnd, text, view->client.x);
		return 0;
	case IDM_EXIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0L);
		return 0;
	case IDM_WIDTH: // assumes that IDM_WHITE
		text->mode = transfer;
		isClassic = ResizeMsg(hwnd, lParam, text, view);
		ClassToTrPos(text);
	case IDM_CLASSIC: // Note: Logic below
		if (isClassic)
		{
			text->mode = classic;
			isClassic = ResizeMsg(hwnd, lParam, text, view);
			TrToClassPos(text);
		}
		SetCaretPos(view->caret.x * view->charSize.x, view->caret.y * view->charSize.y);
		CheckMode(hwnd, &view->iSelection, hMenu, wParam);
		return 0;
	}
	return 0;
}

int SetFocusMsg(HWND hwnd, view_t *view)
{
	CreateCaret(hwnd, NULL, view->charSize.x, view->charSize.y);
	SetCaretPos(view->caret.x * view->charSize.x, view->caret.y * view->charSize.y);
	ShowCaret(hwnd);

	return 0;
}

int KillFocusMsg(HWND hwnd)
{
	HideCaret(hwnd);
	DestroyCaret();

	return 0;
}

/************************************
	Dialog functions (used in main) */

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
