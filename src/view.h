#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED
#include "model.h"

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);
void CheckMode(HWND hwnd, int iSelection, HMENU hMenu, WPARAM wParam);
int ResizeMsg(HWND hwnd, LPARAM lParam, RECT rect, MYTEXT *text, int *iMaxWidth, int *cxClient, int *cyClient, int *curNumLines, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos, int cxChar, int cyChar);

#endif // VIEW_H_INCLUDED
