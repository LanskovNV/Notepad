#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"
#include "header.h"

LPSTR *BuildStrings(LPSTR buffer, int nOfLines, DWORD *width)
{
	int i, j, prewLineEnd = 0;
	DWORD cnt = 0, maxW = 0;

	LPSTR *strings = (CHAR**)calloc(nOfLines, sizeof(CHAR*));
	for (i = 0, j = 0; buffer[i] != '\0'; i++)
	{
		if (buffer[i] == '\n')
		{
			int tmp = 0;

 			if (cnt > maxW)
			{
				maxW = cnt;
				cnt = 0;
			}
			int curStrLen;
			if (prewLineEnd == 0)
			{
				curStrLen = i - 1;
				prewLineEnd--;
			}
			else
				curStrLen = i - prewLineEnd - 2;

			tmp = curStrLen + 1;
			strings[j] = (CHAR*)calloc(tmp <= 1 ? 2 : tmp, sizeof(CHAR));
			if (curStrLen <= 0)
				curStrLen = 1;
			strncpy(strings[j], buffer + prewLineEnd + 1, curStrLen);
			strings[j][curStrLen] = '\0';
			prewLineEnd = i;
			j++;
		}
		cnt++;
	}

	if (i - 1 != prewLineEnd)
	{
		int curStrLen = i - prewLineEnd - 1;

		strings[j] = (CHAR*)calloc(curStrLen + 1, sizeof(CHAR));
		strncpy(strings[j], buffer + prewLineEnd + 1, curStrLen);
		strings[j][curStrLen] = '\0';
		if ((DWORD)curStrLen > maxW)
			maxW = curStrLen;
	}

	*width = maxW;
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

int BuildWidthStrings(MYTEXT *text, DWORD width)
{
	int i, newBufLen = (text->bufLen + 1) * 2;
	LPSTR buffer = text->buffer;
	LPSTR newBuffer = (LPSTR)malloc(newBufLen * sizeof(CHAR));

	ClearString(newBuffer, newBufLen);
	ClearWidthStrings(text);
	for (i = 0; *buffer != '\0'; i++)
	{
		DWORD len1, len2, tmpLen = width;
		LPSTR widthString = (LPSTR)malloc((tmpLen + 1) * sizeof(CHAR));

		ClearString(widthString, tmpLen);
		while (*buffer != '\0')
		{
			while (IsSpace(*buffer) && strlen(widthString) < tmpLen + 1)
			{
				strncat(widthString, buffer, 1);
				buffer++;
			}

			len1 = GetWordLength(buffer);
			len2 = strlen(widthString);
			if (len1 + 1 < width - len2) 
			{
				int length = len1 == width - len2 ? len1 : len1 + 1;
				
				strncat(widthString, buffer, length - 1);                                  
				strncat(widthString, " ", 1);
				if ((int)strlen(buffer) < length)
					buffer += strlen(buffer);
				else
				    buffer += length;
			}
			else if (len1 + 1 >= width && len2 == 0)                                             
			{
				int l = width - 1;
				strncat(widthString, buffer, l);                                    
				buffer += l;
				strncat(widthString, "\n", 1);                                    
				break;
			}
			else
			{
				if (strlen(widthString) < tmpLen)
					strncat(widthString, "\n", 1);
				else
					printf("error\n");
				break;
			}
		}
		strncat(newBuffer, widthString, (strlen(widthString)));
		free(widthString);
	}
	text->numWidthLines = GetNumLines(newBuffer);
	realloc(newBuffer, strlen(newBuffer) + 1);
	text->widthStrings = BuildStrings(newBuffer, text->numWidthLines, &text->curWidth); // ?!?!
	text->curWidth = width;

	free(newBuffer);
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
