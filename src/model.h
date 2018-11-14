#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define FILE_NAME "text1.txt"

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
	DWORD bufLen;
	LPSTR *strings; // all text separated by origin strings
	LPSTR *widthStrings; // 
	LPSTR buffer; // all text
} MYTEXT;

/**
 * functions to work with text
*/
void LoadText(MYTEXT *text, char *fileName, DWORD width);
void OpenFileFunc(HWND hWnd, MYTEXT *text, DWORD width);
LPSTR *BuildStrings(LPSTR buffer, int nOfLines, DWORD *width);
void BuildWidthText(MYTEXT *text, DWORD width, TEXTMETRIC tm);
LPSTR *SelectMode(MYTEXT text);

#endif // MODEL_H_INCLUDED
