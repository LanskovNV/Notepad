#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED

#include "structures.h"

#define FILE_NAME "преступление и наказание.txt"

int LoadText(text_t *text, char *fileName);
int OpenFileFunc(HWND hWnd, text_t *text, int width);
int BuildtrStrings(text_t *text, int width);
int SelectNOfLines(text_t text);
LPSTR *SelectStrings(text_t text);
int ClassToWidePos(text_t *text);
int WideToClassPos(text_t *text);

#endif // MODEL_H_INCLUDED
