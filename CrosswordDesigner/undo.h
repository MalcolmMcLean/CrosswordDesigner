#ifndef undo_h
#define undo_h

void undo_init(void(*enable)(void *ptr, int enableundo, int enableredo), void *ptr);
void undo_push(CROSSWORD *cw);
CROSSWORD *undo_pop(CROSSWORD *cw);
CROSSWORD* undo_redo(void);
int undo_hasundos(void);

#endif
