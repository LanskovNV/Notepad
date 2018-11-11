#include <windows.h>
#include <string.h>
#include "model.h"

static DWORD GetNumLines(LPSTR text)
{
	DWORD cnt = 0;
	int i;

	for (i = 0; text[i] != '\0'; i++)
		if (text[i] == '\n')
			cnt++;

	return i == 0 ? 0 : ++cnt;
}

static LPSTR *ParseBuffer(LPSTR buffer, int nOfLines, DWORD *width)
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

static void ClearText(MYTEXT *text)
{
	int i;

	for (i = 0; i < text->numLines; i++)
		free(text->strings[i]);
	free(text->strings);
	free(text->buffer);
	text->numLines = 0;
	text->strings = NULL;
	text->maxWidth = 0;
}

static DWORD GetWordWidth(LPSTR word, DWORD len, TEXTMETRIC tm)
{
	DWORD ans = 0;


}

void BuildWidthText(MYTEXT *text, DWORD width, TEXTMETRIC tm)
{
	int i;
	text->maxWidth = width;


}
void OpenFileFunc(HWND hWnd, MYTEXT *text)
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

void LoadText(MYTEXT *text, char *fileName)
{
	text->numLines = 0;
	text->maxWidth = 0;
	text->strings = NULL;
	text->WidthStrings = NULL;
	text->buffer = NULL;
	//��������� ����
	HANDLE hFile = CreateFile(fileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	DWORD fileSize = GetFileSize(hFile, NULL);
	OVERLAPPED olf = { 0 }; //���������, � ������� ������ ������� � �����
	//������ LARGE_INTEGER
	//hEvent - ������ NULL
	//Offset ������� ������ ����, ���� ��������� ������. 32-���
	//OffsetHigh - ������� ������ ����, ���� ��������� ������. 32-���
	//Internal - ������ NULL(0). ��� - ������������ ��������, ���� ������������ ����� ���. �������
	//32-���, ������� ������
	//InternalHigh - ���� ��� � ����������, ������ ������� ������

	//��� ���������� � �����������:
	LARGE_INTEGER li = { 0 };
	//LowPart - ������� 32-������ ������
	//HighPart - ������� 32-������ ������
	//QuadPart - 64 ������ �����
	li.QuadPart = 0;
	olf.Offset = li.LowPart;
	olf.OffsetHigh = li.HighPart;
 
	text->buffer = (CHAR*)calloc(fileSize + 1, sizeof(CHAR));
	DWORD iNumRead = 0; //������������ ��������. �������� ���-�� ��������� ����
	
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
		text->numLines = GetNumLines(text->buffer);
		text->strings = ParseBuffer(text->buffer, text->numLines, &text->maxWidth);
		CloseHandle(hFile);
	}	
}