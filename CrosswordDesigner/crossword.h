#ifndef crossword_h
#define crossword_h

typedef struct
{
  char *text;
  int x;
  int y;
} CLUE;

typedef struct
{
  int width;
  int height;
  char *solution;
  unsigned char *grid;
  int *numbers;
  char **wordsacross;
  char **cluesacross;
  int Nacross;
  int *numbersacross;
  char **wordsdown;
  char **cluesdown;
  int Ndown;
  int *numbersdown;
  char **acrossclues_grid;
  char **downclues_grid;
  char *title;
  char *author;
  char *editor;
  char *publisher;
  char *date;
  char *copyright;
} CROSSWORD;

CROSSWORD *createcrossword(int width, int height);
CROSSWORD *crossword_clone(CROSSWORD *cw);
void killcrossword(CROSSWORD *cw);
int crossword_setcell(CROSSWORD *cw, int cx, int cy, char ch);
int crossword_setsolutioncell(CROSSWORD* cw, int cx, int cy, char ch);
int crossword_setacrossclue(CROSSWORD *cw, int id, const char *clue);
int crossword_setdownclue(CROSSWORD *cw, int id, const char *clue);
int crossword_resize(CROSSWORD* cw, int width, int height);
void crossword_startgrid(CROSSWORD *cw);
void crossword_randgrid(CROSSWORD *cw);
int crossword_connected(CROSSWORD *cw);
int crossword_gridsidentical(CROSSWORD* cwa, CROSSWORD* cwb);

#endif
