#include <windows.h>
#include "text.h"

void ClearString(char *s, int len)
{
	int i;

	for (i = 0; i < len; i++)
		*s = 0, s++;
}

BOOL IsSpace(char c)
{
	if (c == '\0' || c == '\r' || c == '\t' || c == '\n' || c == ' ')
		return TRUE;
	else
		return FALSE;
}

DWORD GetNumLines(LPSTR text)
{
	DWORD cnt = 0;
	int i;

	for (i = 0; text[i] != '\0'; i++)
		if (text[i] == '\n')
			cnt++;

	return i == 0 ? 0 : ++cnt;
}

DWORD GetWordLength(LPSTR text)
{
	DWORD cnt = 0;

	while (IsSpace(*text))
		text++;

	while (!IsSpace(*text))
	{
		cnt++;
		text++;
	}

	return cnt;
}

DWORD GetMaxWordLen(LPSTR text)
{
	DWORD cnt = 0, max = 0;

	while (IsSpace(*text))
		text++;

	while (*text != '\0')
	{
		if (IsSpace(*text))
		{
			if (max < cnt)
				max = cnt;
			cnt = 0;
		}
		else
			cnt++;
		text++;
	}

	return max;
}

DWORD NumOfBreaks(LPSTR text, DWORD width)
{
	DWORD cnt = 0;
	DWORD len = 0;

	while (*text != '\0')
	{
		while (IsSpace(*text) && *text != '\0')
			text++;

		if ((len = GetWordLength(text)) > width)
			cnt += len / width;
		text += len;
	}

	return cnt;
}