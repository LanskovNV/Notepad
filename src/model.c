#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

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
		text->pos = 0;
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
