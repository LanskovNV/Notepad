#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "structures.h"

#define IDM_OPEN 1
#define IDM_EXIT 2
#define IDM_CLASSIC 3
#define IDM_WIDTH 4

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int HscrollMsg(mode_t mode, WPARAM wParam, HWND hwnd, view_t *view);
int VscrollMsg(HWND hwnd, WPARAM wParam, view_t *view);
int UpdatePos(HWND hwnd, text_t *text, case_t c, view_t *view);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);
void CheckMode(HWND hwnd, int *iSelection, HMENU hMenu, WPARAM wParam);
int ResizeMsg(HWND hwnd, LPARAM lParam, text_t *text, view_t *view);
int CreateMsg(HWND hwnd, view_t *view);
int PaintMsg(HWND hwnd, text_t *text, view_t *view);
int KeydownMsg(HWND hwnd, WPARAM wParam, text_t *text, view_t *view);
int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, text_t *text, view_t *view);
int SetFocusMsg(HWND hwnd, view_t *view);
int KillFocusMsg(HWND hwnd);


#endif // VIEW_H_INCLUDED
