#include <windows.h>
#include "text.h"

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

