#ifndef fillgrid_h
#define fillgrid_h

#include "crossword.h"

int fillgrid(CROSSWORD* cw, int difficulty, int (*callback)(void* ptr), void* ptr);

#endif