#ifndef undo_h
#define undo_h

void undo_init(void(*enable)(void *ptr), void(*disable)(void *ptr), void *ptr);
void undo_push(CROSSWORD *cw);
CROSSWORD *undo_pop(void);
int undo_hasundos(void);

#endif
