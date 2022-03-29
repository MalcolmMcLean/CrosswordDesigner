#ifndef wordmatcher_h
#define wordmatcher_h

int wordmatcherinit(void);
char **getwordsoflength(int len, int* N);
char **matchword(char *word, int level, int *N);
char **findanagrams(char *word, int level, int *N);
int randword(char *ret, int len, int level);
int wordindictionary(char *word);

#endif
