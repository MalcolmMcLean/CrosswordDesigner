#ifndef crosswordipuz_h
#define crosswordipuz_h

#include "crossword.h"
#include <stdio.h>

CROSSWORD *loadfromipuz(char* fname, int* err);
int saveasipuz(CROSSWORD* cw, char* fname);
int fsaveasipuz(CROSSWORD* cw, FILE *fp);


#endif

