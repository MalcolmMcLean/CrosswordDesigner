#ifndef printcrossowrd_h
#define printcrossword_h

#include "crossword.h"

int PrintCrossword(HWND hwnd, CROSSWORD *cw);
void DrawGrid(HDC hdc, CROSSWORD *cw, int x, int y, int width, int height);
void DrawSolution(HDC hdc, CROSSWORD *cw, int x, int y, int width, int height);

#endif
