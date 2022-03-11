/*******************************************************************
*                         numwin.h                                 *
*            Header for the number control window.                 *
*******************************************************************/

#ifndef numwin_h
#define numwin_h

#define NW_SETMIN (WM_USER + 100)
#define NW_SETMAX (WM_USER + 101)
#define NW_SETDELTA (WM_USER + 102)
#define NW_SETX (WM_USER + 103)

#define NWN_CHANGE 0x100

void RegisterNumWin(HINSTANCE hInstance);
void SetNumWin(HWND hwnd, double x, double min, double max, double delta);
void SetNumWinVal(HWND hwnd, double x);
double GetNumWinVal(HWND hwnd);

#endif                        