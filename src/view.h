#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED
#include "model.h"

#define IDM_OPEN 1
#define IDM_EXIT 2
#define IDM_CLASSIC 3
#define IDM_WIDTH 4

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);
void CheckMode(HWND hwnd, int *iSelection, HMENU hMenu, WPARAM wParam);
int ResizeMsg(HWND hwnd, LPARAM lParam, MYTEXT *text, int *iMaxWidth, int *cxClient, int *cyClient, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos, int cxChar, int cyChar);
int CreateMsg(HWND hwnd, int *cxChar, int *cyChar);
int PaintMsg(HWND hwnd, MYTEXT *text, int iVscrollPos, int iHscrollPos, int cxChar, int cyChar);
int KeydownMsg(HWND hwnd, WPARAM wParam, int *xCaret, int *yCaret, int cxChar, int cyChar, int cxClient, int cyClient);
int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, MYTEXT *text, int *iSelection, int cxChar, int cyChar, int *iMaxWidth, int *cxClient, int *cyClient, int *iVscrollMax, int *iVscrollPos, int *iHscrollMax, int *iHscrollPos);
int HscrollMsg(mode_t mode, WPARAM wParam, HWND hwnd, int *iHscrollPos, int iHscrollMax, int cxChar);
int VscrollMsg(HWND hwnd, WPARAM wParam, int *iVscrollPos, int iVscrollMax, int cyClient, int cyChar);
#endif // VIEW_H_INCLUDED
