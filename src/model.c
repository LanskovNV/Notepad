#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

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

int WideToClassPos(MYTEXT *text)
{
	LPSTR *wideBuf = text->widthStrings;
	LPSTR *classBuf = text->strings;
	pos_t oldPos = text->pos;
	pos_t newPos;
	int i, j;
	int cnt = 0;

	for (i = 0; i <= oldPos.y; i++)
	{
		int curStrLen = strlen(wideBuf[i]);;
		int tmp = i == oldPos.y ? oldPos.x : curStrLen;

		for (j = 0; j < tmp; j++)
		{
			if (j >= tmp)
				break;
			cnt++;
		}
	}

	for (i = 0; cnt > 0; i++)
	{
		int curStrLen = strlen(classBuf[i]);;

		for (j = 0; j < curStrLen && cnt > 0; j++)
			cnt--;
	}
	newPos.x = j;
	newPos.y = i == 0 ? 0 : i - 1;
	text->pos = newPos;

	return 0;
}

int ClassToWidePos(MYTEXT *text)
{
	LPSTR *wideBuf = text->widthStrings;
	LPSTR *classBuf = text->strings;
	pos_t oldPos = text->pos;
	pos_t newPos;
	int i, j;
	int cnt = 0;

	for (i = 0; i <= oldPos.y; i++)
	{
		int curStrLen = strlen(classBuf[i]);;
		int tmp = i == oldPos.y ? oldPos.x : curStrLen;

		for (j = 0; j < tmp; j++)
		{
			if (j >= tmp)
				break;
			cnt++;
		}
	}

	for (i = 0; cnt > 0; i++)
	{
		int curStrLen = strlen(wideBuf[i]);;

		for (j = 0; j < curStrLen && cnt > 0; j++)
			cnt--;
	}

	newPos.x = j;
	newPos.y = i == 0 ? 0 : i - 1;

	text->pos = newPos;
	return 0;
}

static int MoveLeft(pos_t *curPos, pos_t caret, LPSTR *buffer, HWND hwnd, int cxBuffer, int *iHscrollPos, int iHscrollMax, int cxChar, mode_t mode)
{
	int curStringLen = 0;
	int decrease = 1;

	if ((curPos->y > 0 || curPos->x > 0))
	{
		do
		{
			if (caret.x <= 0 && curPos->x != 0)
			{
				int iHscrollInc = -1; 

				decrease = 0;
				*iHscrollPos += iHscrollInc;
				ScrollWindow(hwnd, -cxChar * iHscrollInc, 0, NULL, NULL);
				SetScrollPos(hwnd, SB_HORZ, *iHscrollPos, TRUE);
				curPos->x--;
			}
			else
			{
				if (curPos->x == 0 && curPos->y > 0)
				{
					curPos->y -= 1;
					curStringLen = strlen(buffer[curPos->y]);
					curPos->x = curStringLen - 1;
				}
				else
				{
					curPos->x--;
				}
			}
		} while (curPos->x >= 0 && (curPos->y > 0 || curPos->x > 0) && IsSpace(buffer[curPos->y][curPos->x]));
	}
	return decrease;
}

static int MoveRight(pos_t *curPos, pos_t caret, LPSTR *buffer, HWND hwnd, int cxBuffer, int numLines, int lastLineLen, int *iHscrollPos, int iHscrollMax, int cxChar, mode_t mode)
{
	int curStringLen = strlen(buffer[curPos->y]);
	int increase = 1;

	if ((curPos->y < numLines - 1 || curPos->x < lastLineLen - 1))
	{
		do
		{
			if (caret.x == cxBuffer && curPos->x != curStringLen)
			{
				int iHscrollInc = 1; // cxBuffer * (tmp - inc) + curPos->x % cxBuffer + inc - correct; 

				increase = 0;
				*iHscrollPos += iHscrollInc;
				ScrollWindow(hwnd, -cxChar * iHscrollInc, 0, NULL, NULL);
				SetScrollPos(hwnd, SB_HORZ, *iHscrollPos, TRUE);
				curPos->x++;
			}
			else
			{
				if (curPos->x == curStringLen)
				{
					curPos->y += 1;
					if (curPos->y < numLines)
						curStringLen = strlen(buffer[curPos->y]);
					if (curPos->y != numLines)
						curPos->x = 0;
				}
				else
				{
					curPos->x++;
				}
			}
		} while ((curPos->y < numLines - 1 || curPos->x < lastLineLen - 1) && IsSpace(buffer[curPos->y][curPos->x]));
	}
	return increase;
}

int UpdateClassPos(HWND hwnd, MYTEXT *text, case_t c, int *xCaret, int *yCaret, int cxBuffer, int cyBuffer, int *iHscrollPos, int iHscrollMax, int cxChar)
{
	LPSTR *buffer = SelectStrings(*text);
	pos_t curPos = text->pos;	
	int numLines = SelectNOfLines(*text);
	int lastLineLen = strlen(buffer[numLines - 1]);
	int curStringLen = strlen(buffer[curPos.y]);
	int chX = 1, chY = 1;
	pos_t caret;
	
	caret.x = *xCaret;
	caret.y = *yCaret;

	switch (c)
	{
	case right:
		chX = MoveRight(&curPos, caret, buffer, hwnd, cxBuffer, numLines, lastLineLen, iHscrollPos, iHscrollMax, cxChar, text->mode);
		break;
	case left:
		chX = MoveLeft(&curPos, caret, buffer, hwnd, cxBuffer, iHscrollPos, iHscrollMax, cxChar, text->mode);
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
				MoveLeft(&curPos, caret, buffer, hwnd, cxBuffer, iHscrollPos, iHscrollMax, cxChar, text->mode);
			}
		}
		break;
	case down:
		if (curPos.y < numLines - 1)
		{
			curPos.y += 1;
			curStringLen = strlen(buffer[curPos.y]);
			if (curStringLen <= curPos.x)
				curPos.x = max(0, curStringLen - 1);
			
			if (IsSpace(buffer[curPos.y][curPos.x]))
			{
				if (IsLastSpaces(buffer[curPos.y] + curPos.x, curStringLen - curPos.x, curStringLen))
					MoveLeft(&curPos, caret, buffer, hwnd, cxBuffer, iHscrollPos, iHscrollMax, cxChar, text->mode);
				else 
					MoveRight(&curPos, caret, buffer, hwnd, cxBuffer, numLines, lastLineLen, iHscrollPos, iHscrollMax, cxChar, text->mode);
			}
		}
		break;
	default:
		printf("error in UpdatePos func: incorrect parameter c");
		return 1;
	}
	
	if (chX)
		*xCaret = curPos.x;
	if (chY)
		*yCaret = curPos.y;
	text->pos = curPos;
	

	return 0;
}

int UpdateWidePos()
{
	return 0;
}

LPSTR GenFunc(LPSTR buffer, int *cnt, int *maxL)
{
	LPSTR s = (CHAR*)calloc(*cnt + 1, sizeof(CHAR));

	strncat(s, buffer - *cnt, *cnt);
	strncat(s, "\0", 1);
	*maxL = max(*maxL, *cnt);
	*cnt = 0;

	return s;
}

LPSTR *BuildStrings(LPSTR buffer, int nOfLines, DWORD *width)
{
	int cnt = 0, maxStrLen = 0, j = 0;
	LPSTR *strings = (CHAR**)calloc(nOfLines, sizeof(CHAR*));

	while (*buffer != '\0')
	{
		if (*buffer != '\n')
			cnt++;
		else
			strings[j++] = GenFunc(buffer, &cnt, &maxStrLen);
		buffer++;
		if (*buffer == '\0')
			strings[j] = GenFunc(buffer, &cnt, &maxStrLen);
	} 

	*width = maxStrLen;
	return strings;
}

static void ClearWidthStrings(MYTEXT *text)
{
	int i;

	if (text->numWidthLines != 0)
	{
		for (i = 0; i < (int)text->numWidthLines; i++)
			free(text->widthStrings[i]);
		free(text->widthStrings);
	}
	text->numWidthLines = 0;
}

int BuildWidthStrings(MYTEXT *text, int width)
{
	LPSTR buffer = text->buffer;
	LPSTR newBuffer = (LPSTR)calloc((text->bufLen + 1) * 3, sizeof(CHAR));

	int i;

	ClearWidthStrings(text);
	for (i = 0; *buffer != '\0'; i++)
	{
		int newWidth = width;

		if ((int)strlen(buffer) > newWidth)
			while (!IsSpace(*(buffer + newWidth)) && newWidth != 0)
				newWidth--;
		if (newWidth == 0)
			newWidth = width;

		newWidth = min(newWidth, (int)strlen(buffer));
		strncat(newBuffer, buffer, newWidth);
		strncat(newBuffer, "\r\n", 2);
		buffer += newWidth;
	}
	text->numWidthLines = (int)GetNumLines(newBuffer);
	text->curWidth = width;
	text->widthStrings = BuildStrings(newBuffer, (int)GetNumLines(newBuffer), &width);

	return 0;
}

void LoadText(MYTEXT *text, char *fileName)
{
	text->numLines = 0;
	text->maxWidth = 0;
	text->strings = NULL;
	text->buffer = NULL;
	text->curWidth = 0;
	text->mode = classic;
	text->pos.x = 0;
	text->pos.y = 0;
	//Открываем файл
	HANDLE hFile = CreateFile(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD iNumRead = 0; //Обязательный параметр. получает кол-во считанных байт
	DWORD fileSize = GetFileSize(hFile, NULL);
	OVERLAPPED olf = { 0 }; //Структура, в которой задана позиция в файле
	LARGE_INTEGER li = { 0 };
	li.QuadPart = 0;
	olf.Offset = li.LowPart;
	olf.OffsetHigh = li.HighPart;
 
	text->buffer = (CHAR*)calloc(fileSize + 1, sizeof(CHAR));

	/* TODO: errors detect correctly!!! */
	if (!ReadFile(hFile, text->buffer, fileSize, &iNumRead, &olf))
	{
		CloseHandle(hFile);
	}
	else if (olf.Internal == -1 && GetLastError())
	{
		CloseHandle(hFile);
	}
	else
	{
		text->bufLen = strlen(text->buffer);
		text->maxWordLen = GetMaxWordLen(text->buffer);
		text->numLines = GetNumLines(text->buffer);
		text->strings = BuildStrings(text->buffer, text->numLines, &text->maxWidth);
		CloseHandle(hFile);
	}	
}

static void ClearText(MYTEXT *text)
{
	DWORD i;

	for (i = 0; i < text->numLines; i++)
		free(text->strings[i]);
	if (text->numWidthLines != 0)
	{
		for (i = 0; i < text->numWidthLines; i++)
			free(text->widthStrings[i]);
		free(text->widthStrings);
	}
	text->numWidthLines = 0;
	text->curWidth = 0;
	free(text->strings);	
	free(text->buffer);
	text->numLines = 0;
	text->strings = NULL;
	text->maxWidth = 0;
}

void OpenFileFunc(HWND hWnd, MYTEXT *text, DWORD width)
{
	OPENFILENAME ofn;
	char fileName[100];

	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = fileName;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 100;
	ofn.lpstrFilter = "All files\0*.*\0Text files\0*.txt\0";
	ofn.nFilterIndex = 2; // text files by default

	GetOpenFileName(&ofn);

	ClearText(text);
	LoadText(text, ofn.lpstrFile);

	InvalidateRect(hWnd, NULL, TRUE);
}

LPSTR *SelectStrings(MYTEXT text)
{
	return text.mode == classic ? text.strings : text.widthStrings;
}

DWORD SelectNOfLines(MYTEXT text)
{
	return text.mode == classic ? text.numLines : text.numWidthLines;
}
