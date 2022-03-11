#ifndef crosswordhtml_h
#define crosswordhtml_h

#include <stdio.h>
#include "crossword.h"

int crosswordhtml(char *fname, CROSSWORD *cw, int *err);
int fcrosswordhtml(FILE *fp, CROSSWORD *cw);

int crosswordinteractivehtml(char *fname, CROSSWORD *cw, int *err);
int fcrosswordinteractivehtml(FILE *fp, CROSSWORD *cw);

#endif
