#include <windows.h>
#include <string.h>
#include "model.h"
#include "text.h"

LPSTR *BuildStrings(LPSTR buffer, int nOfLines, DWORD *width)
{
	int i, j, prewLineEnd = 0;
	DWORD cnt = 0, maxW = 0;

	LPSTR *strings = (CHAR**)calloc(nOfLines, sizeof(CHAR*));
	for (i = 0, j = 0; buffer[i] != '\0'; i++)
	{
		if (buffer[i] == '\n')
		{
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

			strings[j] = (CHAR*)calloc(curStrLen + 1, sizeof(CHAR));
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

int BuildWidthStrings(MYTEXT *text, DWORD width, int cxSize)
{
	LPSTR *widthStrings;
	LPSTR buffer = text->buffer;
	int i;
	int nOfLines = text->bufLen * cxSize / width + 2;

	text->curWidth = width;
	if (width < text->maxWordLen * cxSize)
		nOfLines += NumOfBreaks(buffer, width);

	widthStrings = (LPSTR*)calloc(nOfLines, sizeof(CHAR*));
	for (i = 0; i < nOfLines; i++)
	{
		DWORD len1, len2;

		widthStrings[i] = (LPSTR)calloc(width + 1, sizeof(CHAR));
		while (*buffer != '\0')
		{
			while (IsSpace(*buffer) && *buffer != '\0')
				buffer++;

			len1 = GetWordLength(buffer) * cxSize;
			len2 = strlen(widthStrings[i]) * cxSize;
			if (len1 + cxSize <= width - len2) // если слово влезает
			{
				int length = len1 == width - len2 / cxSize ? len1 / cxSize : len1 / cxSize + 1;
				strncat(widthStrings[i], buffer, length);                                   // прибавь его к строке
				buffer += length;
			}
			else if (len1 > width && len2 == 0)                                             // если не влезает в пустую строку
			{
				strncat(widthStrings[i], buffer, width);                                    // копируем сколько влезает 
				buffer += width;
				break;
			}
			else
			{
				break;
			}
		}
	}

	text->numWidthLines = nOfLines;
	text->widthStrings = widthStrings;

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

	//ќткрываем файл
	HANDLE hFile = CreateFile(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD iNumRead = 0; //ќб€зательный параметр. получает кол-во считанных байт
	DWORD fileSize = GetFileSize(hFile, NULL);
	OVERLAPPED olf = { 0 }; //—труктура, в которой задана позици€ в файле
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
	for (i = 0; i < text->numWidthLines; i++)
		free(text->widthStrings[i]);
	free(text->strings);
	free(text->widthStrings);
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

