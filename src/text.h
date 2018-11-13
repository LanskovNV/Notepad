#ifndef _TEXT_H
#define _TEXT_H

DWORD GetNumLines(LPSTR text);
DWORD GetWordLength(LPSTR text);
DWORD GetMaxWordLen(LPSTR text);
DWORD NumOfBreaks(LPSTR text,DWORD width);

#endif
