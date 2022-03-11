#ifndef loadwordlist_h
#define loadwordlist_h

char **loadwordlist(const char *fname, int *N, int *err);
void shufflewordlist(char **list, int N);
void killwordlist(char **list, int N);

#endif
