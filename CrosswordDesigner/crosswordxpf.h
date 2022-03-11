#ifndef crosswordxpf_h
#define crosswordxpf_h

#include <stdio.h>
#include "crossword.h"

int saveasxpf(CROSSWORD *cw, char *fname);
int fsaveasxpf(CROSSWORD *cw, FILE *fp);
CROSSWORD **loadfromxpf(char *fname, int *N, int *err);

#endif
