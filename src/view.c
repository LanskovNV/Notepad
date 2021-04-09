#include <windows.h>
#include <stdio.h>
#include "text.h"
#include "view.h"

/***************************************
	Processing movements (arrows etc.) */

static void FixScrollPos(HWND hwnd, view_t *view, text_t *text, int cxBuffer, int cyBuffer, int chMode)
{
	int deltaX = text->pos.x - (view->iHscrollPos + cxBuffer);
	int deltaY = text->pos.y - (view->iVscrollPos + cyBuffer);
	int scrollSize;
	RECT rect;

	GetClientRect(hwnd, &rect);

	/* fixing horizontal scroll */
	if (deltaX > 0 || deltaX < -cxBuffer || chMode)
	{
		if (deltaX < 0)
		{
			scrollSize = deltaX + cxBuffer;
			view->caret.x = 0;
		}
		else
		{
			view->caret.x = cxBuffer;
			scrollSize = deltaX;
		}
		view->iHscrollPos += scrollSize;

		if (view->iHscrollPos < 0)
		{
			view->iHscrollPos -= scrollSize;
			scrollSize = view->iHscrollPos;
		}
		if (view->iHscrollPos > view->iHscrollMax)
		{
			view->iHscrollPos -= scrollSize;
			scrollSize = view->iHscrollMax - view->iHscrollPos;
		}

		view->caret.x = text->pos.x - view->iHscrollPos;

		ScrollWindow(hwnd, -scrollSize * view->charSize.x, 0, NULL, NULL);
		SetScrollPos(hwnd, SB_HORZ, view->iHscrollPos, TRUE);
		InvalidateRect(hwnd, &rect, TRUE);
		UpdateWindow(hwnd);
	}

	/* fixing vertical scroll */
	if (deltaY > 0 || deltaY < -cyBuffer || chMode)
	{
		if (deltaY < 0)
		{
			scrollSize = deltaY + cyBuffer;
		    view->caret.y = 0;
		}
		else
		{
			scrollSize = deltaY;
			view->caret.y = cyBuffer;
		}
		view->iVscrollPos += scrollSize;

		if (view->iVscrollPos < 0)
		{
			view->iVscrollPos -= scrollSize;
			scrollSize = view->iVscrollPos;
		}
		if (view->iVscrollPos > view->iVscrollMax)
		{
			view->iVscrollPos -= scrollSize;
			scrollSize = view->iVscrollMax - view->iVscrollPos;
		}

		view->caret.y = text->pos.y - view->iVscrollPos;

		ScrollWindow(hwnd, 0, -scrollSize * view->charSize.y, NULL, NULL);
		SetScrollPos(hwnd, SB_VERT, view->iVscrollPos, TRUE);
		InvalidateRect(hwnd, &rect, TRUE);
		UpdateWindow(hwnd);
	}
	ShowCaret(hwnd);
}

static void MoveRight(HWND hwnd, view_t *view, text_t *text)
{
	string_t *buffer = SelectStrings(*text);
	int nOfLines = SelectNOfLines(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);
	int slide = buffer[text->pos.y].string[buffer[text->pos.y].strLen - 1] == '\n' ? SLIDE_SIZE_1 : SLIDE_SIZE_2;

	if (text->pos.x < buffer[text->pos.y].strLen - slide)
	{
		/* changing text pos */
		text->pos.x++;

		/* changing caret pos */
		if (view->caret.x < cxBuffer)
			view->caret.x++;
	}
	else if (text->pos.y < nOfLines - 1)
	{
		/* changing text pos */
		text->pos.x = 0;
		text->pos.y++;

		/* changing caret pos */
		view->caret.x = 0;
		if (view->caret.y < cyBuffer)
			view->caret.y++;
	}

	FixScrollPos(hwnd, view, text, cxBuffer, cyBuffer, NO_CH_MODE);
}

static void MoveLeft(HWND hwnd, view_t *view, text_t *text)
{
	string_t *buffer = SelectStrings(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	if (text->pos.x > 0)
	{
		/* changing text pos */
		text->pos.x--;

		/* changing caret pos */
		if (view->caret.x > 0)
			view->caret.x--;
	}
	else if (text->pos.y > 0)
	{
		int tmp;
		int slide = buffer[text->pos.y - 1].string[buffer[text->pos.y - 1].strLen - 1] == '\n' ? SLIDE_SIZE_1 : SLIDE_SIZE_2;

		/* changing text pos */
		text->pos.y--;
		tmp = buffer[text->pos.y].strLen - slide;
		text->pos.x = tmp > 0 ? tmp : 0;

		/* changing caret pos */
		view->caret.x = text->pos.x;
		if (view->caret.y > 0)
			view->caret.y--;
	}

	FixScrollPos(hwnd, view, text, cxBuffer, cyBuffer, NO_CH_MODE);
}

static void MoveDown(HWND hwnd, view_t *view, text_t *text)
{
	string_t *buffer = SelectStrings(*text);
	int nOfLines = SelectNOfLines(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	if (text->pos.y < nOfLines - 1)
	{
		/* changing text pos */
		int slide = buffer[text->pos.y + 1].string[buffer[text->pos.y + 1].strLen - 1] == '\n' ? SLIDE_SIZE_1 : SLIDE_SIZE_2;
		int tmp = buffer[text->pos.y + 1].strLen - slide;
		int strLen = tmp > 0 ? tmp : 0;

		text->pos.y++;
		if (strLen < text->pos.x)
		{
			text->pos.x = strLen;
			if (text->pos.x >= view->iHscrollPos && text->pos.x < view->iHscrollPos + cxBuffer)
				view->caret.x = text->pos.x;
		}

		/* changing caret pos */
		if (view->caret.y < cyBuffer)
			view->caret.y++;
	}

	FixScrollPos(hwnd, view, text, cxBuffer, cyBuffer, NO_CH_MODE);
}

static void MoveUp(HWND hwnd, view_t *view, text_t *text)
{
	string_t *buffer = SelectStrings(*text);
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	if (text->pos.y > 0)
	{
		/* changing text pos */
		int slide = buffer[text->pos.y - 1].string[buffer[text->pos.y - 1].strLen - 1] == '\n' ? SLIDE_SIZE_1 : SLIDE_SIZE_2;
		int tmp = buffer[text->pos.y - 1].strLen - slide;
		int strLen = tmp > 0 ? tmp : 0;

		text->pos.y--;
		if (strLen < text->pos.x)
		{
			text->pos.x = strLen;
			if (text->pos.x >= view->iHscrollPos && text->pos.x < view->iHscrollPos + cxBuffer)
				view->caret.x = text->pos.x;
		}

		/* changing caret pos */
		if (view->caret.y > 0)
			view->caret.y--;
	}

	FixScrollPos(hwnd, view, text, cxBuffer, cyBuffer, NO_CH_MODE);
}

static int UpdatePos(HWND hwnd, text_t *text, view_t *view, case_t c)
{
	RECT rect;

	GetClientRect(hwnd, &rect);
	switch (c)
	{
	case right:
	    MoveRight(hwnd, view, text);
		break;
	case left:
		MoveLeft(hwnd, view, text);
		break;
	case up:
		MoveUp(hwnd, view, text);
		break;
	case down:
		MoveDown(hwnd, view, text);
		break;
	default:
		printf("error in UpdatePos func: incorrect parameter c");
		return 1;
	}
	return 0;
}

/* debug only
   used to compare num of symbols in different modes */
static void CountSymbols(text_t text)
{
	int i;
	int numOfSymbInClassic = 0;
	int numOfSymbInTransfer = 0;

	for (i = 0; i < text.numClStrings; i++)
		numOfSymbInClassic += text.classic[i].strLen;

	for (i = 0; i < text.numTrStrings; i++)
		numOfSymbInTransfer += text.transfer[i].strLen;

	i += 1;
	return;
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
    // CountSymbols(*text);
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
	case VK_END:
		SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0L);
		break;
    case VK_HOME:
		SendMessage(hwnd, WM_VSCROLL, SB_TOP, 0L);
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

int VscrollMsg(HWND hwnd, WPARAM wParam, view_t *view, text_t *text)
{
	int iVscrollInc = 0;
	int cyBuffer = max(1, view->client.y / view->charSize.y);
	RECT rect;

	GetClientRect(hwnd, &rect);

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
		InvalidateRect(hwnd, &rect, TRUE);
		UpdateWindow(hwnd);
	}
	if (view->caret.y < view->iVscrollPos || view->caret.y > view->iVscrollPos + cyBuffer)
	{
		ShowCaret(hwnd);
		HideCaret(hwnd);
	}

	/*  fix caret pos
		important only when caret pos stay in scroll view
		in other situation caret pos is fixing in other places */
	view->caret.y -= iVscrollInc;

	return 0;
}

int HscrollMsg(my_mode_t mode, WPARAM wParam, HWND hwnd, view_t *view, text_t *text)
{
	int iHscrollInc = 0;
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	RECT rect;

	GetClientRect(hwnd, &rect);

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
			InvalidateRect(hwnd, &rect, TRUE);
			UpdateWindow(hwnd);
		}
	}
	if (view->caret.x < view->iHscrollPos || view->caret.x > view->iHscrollPos + cxBuffer)
	{
		ShowCaret(hwnd);
		HideCaret(hwnd);
	}

	/*  fix caret pos
	important only when caret pos stay in scroll view
	in other situation caret pos is fixing in other places */
	view->caret.x -= iHscrollInc;

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
		if (text->transfer == NULL || text->curWidth != nwidth)
		{
			/* build new transfer strings and recount pos in text */
			if (text->transfer != NULL)
				TrToClassPos(text);
			BuildTrStrings(text, nwidth);
			ClassToTrPos(text);
		}
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

	view->caret.x = text->pos.x - view->iHscrollPos;
	view->caret.y = text->pos.y - view->iVscrollPos;

	ShowCaret(hwnd);
	HideCaret(hwnd);

	return isClassic;
}

int PaintMsg(HWND hwnd, text_t *text, view_t *view)
{
	PAINTSTRUCT ps;
	string_t *curStrings = NULL;
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
			curStrings[i].string,
			curStrings[i].strLen
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
	int cxBuffer = max(1, view->client.x / view->charSize.x);
	int cyBuffer = max(1, view->client.y / view->charSize.y);

	switch (LOWORD(wParam))
	{
	case IDM_OPEN:
		OpenFileFunc(hwnd, text, view->client.x);
		view->caret.x = 0;
		view->caret.y = 0;
		view->iSelection = IDM_CLASSIC;
		text->pos.x = 0;
		text->pos.y = 0;
		view->iHscrollPos = 0;
		view->iVscrollPos = 0;
		text->mode = classic;
		CheckMenuItem(hMenu, IDM_WIDTH, MF_UNCHECKED);
		CheckMenuItem(hMenu, IDM_CLASSIC, MF_CHECKED);
		ResizeMsg(hwnd, lParam, text, view);
		return 0;
	case IDM_EXIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0L);
		return 0;
	case IDM_WIDTH: // assumes that IDM_WHITE
		text->mode = transfer;
		isClassic = ResizeMsg(hwnd, lParam, text, view);
	case IDM_CLASSIC: // Note: Logic below
		if (isClassic)
		{
			text->mode = classic;
			isClassic = ResizeMsg(hwnd, lParam, text, view);
			TrToClassPos(text);
		}
		SetCaretPos(view->caret.x * view->charSize.x, view->caret.y * view->charSize.y);
		CheckMode(hwnd, &view->iSelection, hMenu, wParam);
		FixScrollPos(hwnd, view, text, cxBuffer, cyBuffer, IS_CH_MODE);
		HideCaret(hwnd);
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
