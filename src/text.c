#include <windows.h>
#include <ctype.h>
#include "text.h"

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

	while (isspace(*text))
		text++;

	while (!isspace(*text))
	{
		cnt++;
		text++;
	}

	return cnt;
}

DWORD GetMaxWordLen(LPSTR text)
{
	DWORD cnt = 0, max = 0;

	while (isspace(*text))
		text++;

	while (*text != '\0')
	{
		if (isspace(*text))
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
		while (isspace(*text) && *text != '\0')
			text++;

		if ((len = GetWordLength(text)) > width)
			cnt += len / width;
		text += len;
	}

	return cnt;
}