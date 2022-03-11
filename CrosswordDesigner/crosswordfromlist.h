#ifndef crosswordfromlist_h
#define crosswordfromlist_h

void crosswordfromlist(char *puzzle, int width, int height, char **dict, int N, int iter, int quiet);
int *generatenumbers(char *puzzle, int width, int height);
unsigned long fletcher(char *data, int len );

#endif
