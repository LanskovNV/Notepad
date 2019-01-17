#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "model.h"

#define IDM_OPEN 1
#define IDM_EXIT 2
#define IDM_CLASSIC 3
#define IDM_WIDTH 4

/* struct to display text */
typedef struct tagView {
	pos_t client;       /* screen size */
	pos_t charSize;
	pos_t caret;        /* current pos on screen */
	int iMaxWidth;      /* max Hscroll length */
	int iVscrollMax;
	int iVscrollPos;
	int iHscrollMax;
	int iHscrollPos;
	int iSelection;    /* active menu flag (mode) */
} view_t;

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);

int ResizeMsg(HWND hwnd, LPARAM lParam, text_t *text, view_t *view);
int CreateMsg(HWND hwnd, view_t *view);
int PaintMsg(HWND hwnd, text_t *text, view_t *view);
int KeydownMsg(HWND hwnd, WPARAM wParam, text_t *text, view_t *view);
int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, text_t *text, view_t *view);
int SetFocusMsg(HWND hwnd, view_t *view);
int KillFocusMsg(HWND hwnd);
int HscrollMsg(mode_t mode, WPARAM wParam, HWND hwnd, view_t *view);
int VscrollMsg(HWND hwnd, WPARAM wParam, view_t *view);

#endif // VIEW_H_INCLUDED
