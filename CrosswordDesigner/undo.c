#include <stdlib.h>
#include <string.h>
#include "crossword.h"
#include "undo.h"

typedef struct
{
	CROSSWORD **crosswords;
	int N;
	void(*enable)(void *ptr);
	void(*disable)(void *ptr);
	void *ptr;

} UNDO;

static UNDO undo_buffer =
{
	0, 0, 0, 0, 0
};

void undo_init(void(*enable)(void *ptr), void(*disable)(void *ptr), void *ptr)
{
	undo_buffer.crosswords = 0;
	undo_buffer.N = 0;
	undo_buffer.enable = enable;
	undo_buffer.disable = disable;
	undo_buffer.ptr = ptr;
	if (undo_buffer.disable)
		(*undo_buffer.disable)(ptr);
}
void undo_push(CROSSWORD *cw)
{
	CROSSWORD **temp;

	if (undo_buffer.N == 1000)
	{
		killcrossword(undo_buffer.crosswords[0]);
		memmove(&undo_buffer.crosswords[0], &undo_buffer.crosswords[1], 999 * sizeof(CROSSWORD *));
		undo_buffer.crosswords[999] = crossword_clone(cw);
	}
	else
	{
		temp = realloc(undo_buffer.crosswords, (undo_buffer.N + 1) * sizeof(CROSSWORD *));
		if (!temp)
			return;
		undo_buffer.crosswords = temp;
		undo_buffer.crosswords[undo_buffer.N] = crossword_clone(cw);
		undo_buffer.N++;
		if (undo_buffer.N == 1)
			if (undo_buffer.enable)
				(*undo_buffer.enable)(undo_buffer.ptr);

	}

}

CROSSWORD *undo_pop(void)
{
	if (undo_buffer.N)
	{
		if (undo_buffer.N == 1)
			if (undo_buffer.disable)
				(*undo_buffer.disable)(undo_buffer.ptr);
		return undo_buffer.crosswords[--undo_buffer.N];
	}
	else
		return 0;
}

int undo_hasundos(void)
{
	return undo_buffer.N;
}