#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define FILE_NAME "преступление и наказание.txt"

typedef enum {
	classic,
	width
} mode_t;

/**
 * struct to work with text
*/
typedef struct tagMYTEXT {
	mode_t mode;
	DWORD numLines;
	DWORD curWidth;
	DWORD numWidthLines;
	DWORD maxWidth;
	DWORD maxWordLen;
	DWORD bufLen;
	LPSTR *strings; // all text separated by origin strings
	LPSTR *widthStrings; // 
	LPSTR buffer; // all text
} MYTEXT;

/**
 * functions to work with text
*/
void LoadText(MYTEXT *text, char *fileName);
void OpenFileFunc(HWND hWnd, MYTEXT *text, DWORD width);
int BuildWidthStrings(MYTEXT *text, DWORD width, int cxSize);
DWORD SelectNOfLines(MYTEXT text);
LPSTR *SelectStrings(MYTEXT text);

#endif // MODEL_H_INCLUDED
