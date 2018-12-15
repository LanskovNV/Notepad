#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

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

int WideToClassPos(MYTEXT *text, int *xCaret, int *yCaret)
{
	LPSTR *wideBuf = text->widthStrings;
	LPSTR *classBuf = text->strings;
	pos_t oldPos = text->pos;
	pos_t newPos;
	int i, j;
	int cnt;

	for (i = 0; i <= oldPos.y; i++)
	{
		int curStrLen = strlen(wideBuf[i]);;

		for (j = 0; j < i == oldPos.y ? oldPos.x + 1 : curStrLen; j++)
			cnt++;
	}

	for (i = 0; cnt > 0; i++)
	{
		int curStrLen = strlen(classBuf[i]);;

		for (j = 0; j < curStrLen && cnt > 0; j++)
			cnt--;
	}

	newPos.x = j;
	newPos.y = i;

	text->pos = newPos;
	*xCaret = newPos.x;
	*yCaret = newPos.y;

	return 0;
}

int ClassToWidePos(MYTEXT *text, int *xCaret, int *yCaret)
{
	LPSTR *wideBuf = text->widthStrings;
	LPSTR *classBuf = text->strings;
	pos_t oldPos = text->pos;
	pos_t newPos;
	int i, j;
	int cnt;

	for (i = 0; i <= oldPos.y; i++)
	{
		int curStrLen = strlen(classBuf[i]);;

		for (j = 0; j < i == oldPos.y ? oldPos.x + 1 : curStrLen; j++)
			cnt++;
	}

	for (i = 0; cnt > 0; i++)
	{
		int curStrLen = strlen(wideBuf[i]);;

		for (j = 0; j < curStrLen && cnt > 0; j++)
			cnt--;
	}

	newPos.x = j;
	newPos.y = i;

	text->pos = newPos;
	*xCaret = newPos.x;
	*yCaret = newPos.y;

	return 0;
}

static int MoveLeft(pos_t *curPos , LPSTR *buffer, HWND hwnd, int cxBuffer)
{
	int curStringLen = 0;

	if ((curPos->y > 0 || curPos->x > 0))
	{
		do
		{
			if (curPos->x == 0 && curPos->x - cxBuffer > 0)
			{
				SendMessage(hwnd, WM_HSCROLL, -1, 0L);
				curPos->x--;
			}
			else if (curPos->x == 0 && curPos->y > 0)
			{
				curPos->y -= 1;
				curStringLen = strlen(buffer[curPos->y]);
				curPos->x = curStringLen - 1;
			}
			else
			{
				curPos->x--;
			}
		} while (curPos->x >= 0 && (curPos->y > 0 || curPos->x > 0) && IsSpace(buffer[curPos->y][curPos->x]));
	}
	return 0;
}

static int MoveRight(pos_t *curPos, LPSTR *buffer, HWND hwnd, int cxBuffer, int numLines, int lastLineLen)
{
	int curStringLen = strlen(buffer[curPos->y]);

	if ((curPos->y < numLines - 1 || curPos->x < lastLineLen - 1))
	{
		do
		{
			if (curPos->x == cxBuffer - 1)
			{
				SendMessage(hwnd, WM_HSCROLL, 1, 0L);
				curPos->x++;
			}
			else if (curPos->x == curStringLen)
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
		} while (curPos->x <= cxBuffer && (curPos->y < numLines - 1 || curPos->x < lastLineLen - 1) && IsSpace(buffer[curPos->y][curPos->x]));
	}
	return 0;
}

int UpdateClassPos(HWND hwnd, MYTEXT *text, case_t c, int *xCaret, int *yCaret, int cxBuffer, int cyBuffer)
{
	LPSTR *buffer = text->strings;
	pos_t curPos = text->pos;	
	int lastLineLen = strlen(buffer[text->numLines - 1]);
	int curStringLen = strlen(buffer[curPos.y]);

	/* TODO: fix scroll !!! */

	switch (c)
	{
	case right:
		MoveRight(&curPos, buffer, hwnd, cxBuffer, text->numLines, lastLineLen);
		break;
	case left:
		MoveLeft(&curPos, buffer, hwnd, cxBuffer);
		break;
	case up:
		if (curPos.y > 0)
		{
			if (curPos.y - cyBuffer > 0)
			{
				SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0L);
			}
			curPos.y -= 1;
			curStringLen = strlen(buffer[curPos.y]);
			if (curPos.x > curStringLen)
				curPos.x = curStringLen - 1;
			
			if (IsSpace(buffer[curPos.y][curPos.x]))
			{
				MoveLeft(&curPos, buffer, hwnd, cxBuffer);
			}
		}
		break;
	case down:
		if (curPos.y < text->numLines - 1)
		{
			if (cyBuffer - curPos.y - 1 == 0)
			{
				SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0L);
			}
			curPos.y += 1;
			curStringLen = strlen(buffer[curPos.y]);
			if (curStringLen <= curPos.x)
				curPos.x = max(0, curStringLen - 1);
			
			if (IsSpace(buffer[curPos.y][curPos.x]))
			{
				if (IsLastSpaces(buffer[curPos.y] + curPos.x, curStringLen - curPos.x, curStringLen))
					MoveLeft(&curPos, buffer, hwnd, cxBuffer);
				else 
					MoveRight(&curPos, buffer, hwnd, cxBuffer, text->numLines, lastLineLen);
			}
		}
		break;
	default:
		printf("error in UpdatePos func: incorrect parameter c");
		return 1;
	}
	
	*xCaret = curPos.x;
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
	LPSTR newBuffer = (LPSTR)calloc((text->bufLen + 1) * 2, sizeof(CHAR));

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
		strncat(newBuffer, "\n", 1);
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
