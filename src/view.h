#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);
void CheckMode(HWND hwnd, int iSelection, HMENU hMenu, WPARAM wParam);

#endif // VIEW_H_INCLUDED
