#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#define FILE_NAME "преступление и наказание.txt"

typedef enum {
	classic,
	width
} mode_t;

typedef enum {
	right,
	left,
	up,
	down
} case_t;

typedef struct {
	int x;
	int y;
} pos_t;

/**
 * struct to work with text
*/
typedef struct tagMYTEXT {
	mode_t mode;
	pos_t pos;
	DWORD numLines;
	DWORD curWidth;
	DWORD numWidthLines;
	DWORD maxWidth;
	DWORD maxWordLen;
	DWORD bufLen;
	LPSTR *strings; 
	LPSTR *widthStrings; 
	LPSTR buffer; 
} MYTEXT;

/**
 * functions to work with text
*/
void LoadText(MYTEXT *text, char *fileName);
void OpenFileFunc(HWND hWnd, MYTEXT *text, DWORD width);
int BuildWidthStrings(MYTEXT *text, int width);
DWORD SelectNOfLines(MYTEXT text);
LPSTR *SelectStrings(MYTEXT text);
int UpdateClassPos(HWND hwnd, MYTEXT *text, case_t c, int *xCaret, int *yCaret, int cxBuffer, int cyBuffer);

#endif // MODEL_H_INCLUDED
