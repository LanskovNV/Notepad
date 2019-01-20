#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

/***************** checked 17.01.2019
	source funcs */

static void ClearText(text_t *text)
{
	text->numTrStrings = 0;
	text->numClStrings = 0;
	text->maxWidth = 0;
	text->curWidth = 0;
	free(text->buffer);
	free(text->classic);
	free(text->transfer);
}

static string_t *BuildStrings(LPSTR buffer, int nOfStrings, int *maxWidth)
{
	string_t *strings = malloc(sizeof(string_t) * nOfStrings);
	int cntSymb = 0;
	int cntStr = 0;
	int max = 0;

	while (cntStr < nOfStrings)
	{
		while (*buffer != '\n' && *buffer != '\0')
		{
			cntSymb++;
			buffer++;
		}
		strings[cntStr].strLen = cntSymb;
		strings[cntStr].string = buffer - cntSymb;
		if (cntSymb > max)
			max = cntSymb;
		cntSymb = 0;
		cntStr++;
		buffer++;
	}
	*maxWidth = max;

	return strings;
}

/************************** checked 17.01.2019
	main text logic funcs */

int TrToClassPos(text_t *text)
{
	return 0;
}

int ClassToTrPos(text_t *text)
{
	return 0;
}

int BuildTrStrings(text_t *text, int width) // width in symbols
{
	int numOfStrings = (text->bufLen / width + 1) * 2;
	string_t *transfer = malloc(sizeof(string_t) * numOfStrings);
	LPSTR buffer = text->buffer;
	int cntStr = 0;
	int cntBuf = 0;
	int cntSymb = width;

	while (cntBuf + width < text->bufLen)
	{
		if (!IsSpace(*(buffer + width)) && !IsSpace(*(buffer + width + 1)))
		{
			int i;

			for (i = width; i > 0 && !IsSpace(*(buffer + i)); i--)	{}
			if (i != 0)	cntSymb = i;
		}
		transfer[cntStr].string = buffer;
		transfer[cntStr].strLen = cntSymb;
		buffer += cntSymb;
		cntStr++;
		cntBuf += cntSymb;
		cntSymb = width;
	}
	if (cntBuf != text->bufLen)
	{
		cntSymb = text->bufLen - cntBuf;
		transfer[cntStr].string = buffer + cntSymb;
		transfer[cntStr].strLen = cntSymb;
		cntStr++;
	}
	text->curWidth = width;
	text->numTrStrings = cntStr;
	realloc(transfer, sizeof(string_t) * cntStr);
	text->transfer = transfer;

	return 0;
}

int LoadText(text_t *text, char *fileName)
{
	text->numClStrings = 0;
	text->numTrStrings = 0;
	text->maxWidth = 0;
	text->curWidth = 0;
	text->bufLen = 0;
	text->buffer = NULL;
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
		text->numClStrings = GetNumLines(text->buffer);
		text->classic = BuildStrings(text->buffer, text->numClStrings, &text->maxWidth);
		text->curWidth = text->maxWidth;
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

int SelectNOfLines(text_t text)
{
	return text.mode == classic ? text.numClStrings : text.numTrStrings;
}

string_t *SelectStrings(text_t text)
{
	return text.mode == classic ? text.classic : text.transfer;
}
