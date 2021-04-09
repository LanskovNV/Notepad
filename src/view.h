#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

#include "model.h"

#define IDM_OPEN 1
#define IDM_EXIT 2
#define IDM_CLASSIC 3
#define IDM_WIDTH 4
#define IS_CH_MODE 5
#define NO_CH_MODE 0

/*this magic numbers used for skipping \r and \n symbols in text with caret*/
#define SLIDE_SIZE_1 3
#define SLIDE_SIZE_2 1

typedef enum {
	false,
	true
}bool_t;

/* struct to display text */
typedef struct tagView {
	pos_t client;       /* screen size in pixels*/
	pos_t charSize;     /* in pixels */
	pos_t caret;        /* current pos on screen in symbols*/
	int iMaxWidth;      /* max Hscroll length */
	int iVscrollMax;
	int iVscrollPos;
	int iHscrollMax;
	int iHscrollPos;
	bool_t isCarHidden;
	int iSelection;     /* menu selector: classic or transfer (mode) */
} view_t;

int ResizeMsg(HWND hwnd, LPARAM lParam, text_t *text, view_t *view);
int CreateMsg(HWND hwnd, view_t *view);
int PaintMsg(HWND hwnd, text_t *text, view_t *view);
int KeydownMsg(HWND hwnd, WPARAM wParam, text_t *text, view_t *view);
int CommandMsg(HWND hwnd, WPARAM wParam, LPARAM lParam, text_t *text, view_t *view);
int SetFocusMsg(HWND hwnd, view_t *view);
int KillFocusMsg(HWND hwnd);
int HscrollMsg(my_mode_t mode, WPARAM wParam, HWND hwnd, view_t *view, text_t *text);
int VscrollMsg(HWND hwnd, WPARAM wParam, view_t *view, text_t *text);

#endif // VIEW_H_INCLUDED
