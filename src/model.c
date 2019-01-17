#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

/*****************
	source funcs */
static void ClearTrStrings(text_t *text)
{
	int i;

	if (text->numTrStrings != 0)
	{
		for (i = 0; i < (int)text->numTrStrings; i++)
			free(text->trStrings[i]);
		free(text->trStrings);
	}
	text->numTrStrings = 0;
}

static void ClearText(text_t *text)
{
	int i;

	for (i = 0; i < text->numClStrings; i++)
		free(text->clStrings[i]);
	if (text->numTrStrings != 0)
	{
		for (i = 0; i < text->numTrStrings; i++)
			free(text->trStrings[i]);
		free(text->trStrings);
	}
	text->numTrStrings = 0;
	text->curWidth = 0;
	free(text->clStrings);
	free(text->buffer);
	text->numClStrings = 0;
	text->clStrings = NULL;
	text->maxWidth = 0;
}

static LPSTR GenNewString(LPSTR buffer, int *cnt, int *maxL)
{
	LPSTR s = (CHAR*)calloc(*cnt + 1, sizeof(CHAR));

	strncat(s, buffer - *cnt, *cnt);
	strcat(s, "\0");
	*maxL = max(*maxL, *cnt);
	*cnt = 0;

	return s;
}

static LPSTR *BuildStrings(LPSTR buffer, int nOfLines, int *width)
{
	int cnt = 0, maxStrLen = 0, j = 0;
	LPSTR *clStrings = (CHAR**)calloc(nOfLines, sizeof(CHAR*));

	while (*buffer != '\0')
	{
		if (*buffer != '\n')
			cnt++;
		else
			clStrings[j++] = GenNewString(buffer, &cnt, &maxStrLen);
		buffer++;
	}
	clStrings[j] = GenNewString(buffer, &cnt, &maxStrLen);

	*width = maxStrLen;
	return clStrings;
}

/**************************
	main text logic funcs */

int WideToClassPos(text_t *text)
{
	LPSTR *wideBuf = text->trStrings;
	LPSTR *classBuf = text->clStrings;
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

int ClassToWidePos(text_t *text)
{
	LPSTR *wideBuf = text->trStrings;
	LPSTR *classBuf = text->clStrings;
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

int BuildtrStrings(text_t *text, int width)
{
	LPSTR buffer = text->buffer;
	LPSTR newBuffer = (LPSTR)calloc((text->bufLen + 1) * 3, sizeof(CHAR));

	int i;

	ClearTrStrings(text);
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
	text->numTrStrings = (int)GetNumLines(newBuffer);
	text->curWidth = width;
	text->trStrings = BuildStrings(newBuffer, text->numTrStrings, &width);

	return 0;
}

int SelectNOfLines(text_t text)
{
	return text.mode == classic ? text.numClStrings : text.numTrStrings;
}

int LoadText(text_t *text, char *fileName)
{
	text->numClStrings = 0;
	text->maxWidth = 0;
	text->clStrings = NULL;
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

	int iNumRead = 0; //Обязательный параметр. получает кол-во считанных байт
	int fileSize = GetFileSize(hFile, NULL);
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
		return 1;
	}
	else if (olf.Internal == -1 && GetLastError())
	{
		CloseHandle(hFile);
		return 2;
	}
	else
	{
		text->bufLen = strlen(text->buffer);
		// text->maxWordLen = GetMaxWordLen(text->buffer);
		text->numClStrings = GetNumLines(text->buffer);
		text->clStrings = BuildStrings(text->buffer, text->numClStrings, &text->maxWidth);
		CloseHandle(hFile);
		return 0;
	}
}

int OpenFileFunc(HWND hWnd, text_t *text, int width)
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

	return 0;
}

LPSTR *SelectStrings(text_t text)
{
	return text.mode == classic ? text.clStrings : text.trStrings;
}
