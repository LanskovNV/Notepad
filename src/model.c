#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "model.h"
#include "text.h"

/***************** checked 17.01.2019
	source funcs */

void ClearText(text_t *text)
{
	text->numTrStrings = 0;
	text->numClStrings = 0;
	text->maxWidth = 0;
	text->curWidth = 0;
	if (text->transfer != NULL)
		free(text->transfer);
	free(text->classic);
	free(text->buffer);
}

/* free transfer if exists */
static void ClearStrings(text_t *text)
{
	if (text->transfer != NULL && text->numTrStrings != 0)
	{
		free(text->transfer);
		text->transfer = NULL;
	}
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
		if (*buffer != '\0')
			strings[cntStr].strLen = cntSymb + 1;
		else
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
    int posInBuffer = 0;
    int i;
    pos_t newPos;

    newPos.x = 0;
    newPos.y = 0;
    // find out pos in buffer
    for (i = 0; i <= text->pos.y; i++)
    {
        if (i == text->pos.y)
            posInBuffer += text->pos.x;
        else
            posInBuffer += text->transfer[i].strLen;
    }
    // find pos in class
    i = 0;
    while (posInBuffer > 0)
    {
		if (posInBuffer < text->classic[i].strLen)
		{
			newPos.x = posInBuffer;
			posInBuffer = 0;
		}
        else
        {
            posInBuffer -= text->classic[i].strLen;
            i++;
            newPos.y++;
        }
    }
    text->pos = newPos;
    return 0;
}

int ClassToTrPos(text_t *text)
{
        int posInBuffer = 0;
    int i;
    pos_t newPos;

    newPos.x = 0;
    newPos.y = 0;
    // find out pos in buffer
    for (i = 0; i <= text->pos.y; i++)
    {
        if (i == text->pos.y)
            posInBuffer += text->pos.x;
        else
            posInBuffer += text->classic[i].strLen;
    }

    // find pos in class
    i = 0;
    while (posInBuffer > 0)
    {
		if (posInBuffer < text->transfer[i].strLen)
		{
			newPos.x = posInBuffer;
			posInBuffer = 0;
		}
        else
        {
            posInBuffer -= text->transfer[i].strLen;
            i++;
            newPos.y++;
        }

    }

    text->pos = newPos;
    return 0;
}

int BuildTrStrings(text_t *text, int width) // width in symbols
{
	string_t *transfer = malloc(sizeof(string_t) * text->bufLen);
	int cntStr = 0;
	int j;

	ClearStrings(text);
	for (j = 0; j < text->numClStrings; j++)
	{
		if (text->classic[j].strLen < width)
		{
			transfer[cntStr].string = text->classic[j].string;
			transfer[cntStr].strLen = text->classic[j].strLen;
			cntStr++;
		}
		else
		{
			int k, cntSymb = width;
			LPSTR buffer = text->classic[j].string;
			int lastLen = text->classic[j].strLen;

			for (k = 0; lastLen != 0; k++)
			{
				if (lastLen > width && !IsSpace(*(buffer + width - 1)) && !IsSpace(*(buffer + width)))
				{
					int i = cntSymb;

					while (i > 0 && !IsSpace(*(buffer + i)))
						i--;
					if (i != 0)
						cntSymb = i;
				}
				transfer[cntStr].string = buffer;
				if (lastLen >= width)
				{
					transfer[cntStr].strLen = cntSymb;
					buffer += cntSymb;
					lastLen -= cntSymb;
				}
				else
				{
					transfer[cntStr].strLen = lastLen;
					buffer += lastLen;
					lastLen = 0;
				}
				cntStr++;
				cntSymb = width;
			}
		}
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
	    printf("error 1 in load file");
		CloseHandle(hFile);
		return 1;
	}
	else if (olf.Internal == -1 && GetLastError())
	{
	    printf("error 2 in load file");
		CloseHandle(hFile);
		return 2;
	}
	else
	{
		text->bufLen = strlen(text->buffer);
		text->numClStrings = GetNumLines(text->buffer);
		text->classic = BuildStrings(text->buffer, text->numClStrings, &text->maxWidth);
		text->curWidth = text->maxWidth;
		text->transfer = NULL;
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
