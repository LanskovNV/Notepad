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
	LPSTR *strings = (LPSTR*)calloc(text->bufLen,sizeof(LPSTR*)); /* строк не больше чем символов 100% */
	int i;

	ClearWidthStrings(text);
	for (i = 0; *buffer != '\0'; i++)
	{
		int newWidth = width;

		/* переместись назад до пробела */
		if ((int)strlen(buffer) > newWidth)
			while (!IsSpace(*(buffer + newWidth)) && newWidth != 0)
				newWidth--;
		if (newWidth == 0)
			newWidth = width;
		/* скопируй строку */
		strings[i] = (LPSTR)calloc(newWidth + 1, sizeof(CHAR));
		strncpy(strings[i], buffer, newWidth);
		buffer += newWidth;
	}
	realloc(strings, i * sizeof(LPSTR*));
	text->numWidthLines = i;
	text->curWidth = width;
	text->widthStrings = strings;

	return 0;
}
/*
int BuildWidthStrings(MYTEXT *text, DWORD width)
{
	int i, newBufLen = (text->bufLen + 1) * 2;
	LPSTR buffer = text->buffer;
	LPSTR newBuffer = (LPSTR)malloc(newBufLen * sizeof(CHAR));

	ClearString(newBuffer, newBufLen);
	ClearWidthStrings(text);
	if ((int)text->maxWidth <= width)
	{
		text->widthStrings = (CHAR**)calloc(text->numLines, sizeof(CHAR*));
		for (i = 0; i < (int)text->numLines; i++)
		{
			text->widthStrings[i] = (CHAR*)calloc(strlen(text->strings[i]) + 1, sizeof(CHAR));
			strcpy(text->widthStrings[i], text->strings[i]);
		}
		text->curWidth = width;
		text->numWidthLines = text->numLines;
	}
	else
	{
		for (i = 0; *buffer != '\0'; i++)
		{
			DWORD len1, len2, tmpLen = width;
			LPSTR widthString = (LPSTR)malloc((tmpLen + 1) * sizeof(CHAR));

			ClearString(widthString, tmpLen);
			while (*buffer != '\0')
			{
				while (IsSpace(*buffer) && strlen(widthString) < tmpLen)
				{
					strncat(widthString, buffer, 1);
					buffer++;
				}

				if (IsSpace(*buffer))
				{
					break;
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
	}
	free(newBuffer);
	return 0;
}
*/

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
