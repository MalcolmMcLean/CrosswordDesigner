#ifndef crosswordpuz_h
#define crosswordpuz_h

CROSSWORD *loadfrompuz(char *fname, int *err);
CROSSWORD *floadfrompuz(FILE *fp, int *err);
int saveaspuz(CROSSWORD *cw, char *fname);
int fsaveaspuz(CROSSWORD *cw, FILE *fp);

#endif
