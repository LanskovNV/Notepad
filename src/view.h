#ifndef VIEW_H_INCLUDED
#define VIEW_H_INCLUDED

LRESULT CALLBACK DialogProcedure(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void RegisterDialogClass(HINSTANCE hInst);
void DisplayDialog(HWND hwnd);

#endif // VIEW_H_INCLUDED
