#include <stdlib.h>
#include <string.h>
#include "crossword.h"
#include "undo.h"

typedef struct
{
	CROSSWORD **crosswords;
	int N;
	int undotop;
	void(*enable)(void *ptr, int enableudo, int enableredo);
	void *ptr;

} UNDO;

static UNDO undo_buffer =
{
	0, 0, 0, 0, 0
};

void undo_init(void(*enable)(void* ptr, int enableudo, int enableredo), void* ptr)
{
	undo_buffer.crosswords = 0;
	undo_buffer.N = 0;
	undo_buffer.undotop = 0;
	undo_buffer.enable = enable;
	undo_buffer.ptr = ptr;
	if (undo_buffer.enable)
		(*undo_buffer.enable)(ptr, 0, 0);
}

void undo_push(CROSSWORD *cw)
{
	CROSSWORD **temp;
	int i;

	if (undo_buffer.N > undo_buffer.undotop)
	{
		for (i = undo_buffer.undotop; i < undo_buffer.N; i++)
		{
			killcrossword(undo_buffer.crosswords[i]);
			undo_buffer.crosswords[i] = 0;
		}
		undo_buffer.N = undo_buffer.undotop;
	}

	if (undo_buffer.N == 1000)
	{
		killcrossword(undo_buffer.crosswords[0]);
		memmove(&undo_buffer.crosswords[0], &undo_buffer.crosswords[1], 999 * sizeof(CROSSWORD *));
		undo_buffer.crosswords[999] = crossword_clone(cw);
	}
	else
	{
		if (undo_buffer.undotop == undo_buffer.N)
		{
			temp = realloc(undo_buffer.crosswords, (undo_buffer.N + 1) * sizeof(CROSSWORD*));
			if (!temp)
				return;
			undo_buffer.crosswords = temp;
			undo_buffer.N++;
		}
		undo_buffer.crosswords[undo_buffer.undotop] = crossword_clone(cw);
		undo_buffer.undotop++;
	}
	if (undo_buffer.enable)
		(*undo_buffer.enable)(undo_buffer.ptr, undo_hasundos(), undo_hasredos());

}

CROSSWORD *undo_pop(CROSSWORD *cw)
{
	CROSSWORD* answer = 0;

	if (undo_buffer.undotop == undo_buffer.N)
	{
		if (undo_buffer.N == 1000)
		{
			killcrossword(undo_buffer.crosswords[0]);
			memmove(&undo_buffer.crosswords[0], &undo_buffer.crosswords[1], 999 * sizeof(CROSSWORD*));
			undo_buffer.crosswords[999] = crossword_clone(cw);
			undo_buffer.undotop = 999;
		}
		else
		{
			void* temp = realloc(undo_buffer.crosswords, (undo_buffer.N + 1) * sizeof(CROSSWORD*));
			if (!temp)
				return 0;
			undo_buffer.crosswords = temp;
			undo_buffer.crosswords[undo_buffer.N++] = crossword_clone(cw);
		}
	}

	if (undo_buffer.undotop)
	{
		answer = crossword_clone(undo_buffer.crosswords[--undo_buffer.undotop]);
	}
	if (undo_buffer.enable)
	{
		(*undo_buffer.enable)(undo_buffer.ptr, undo_hasundos(), undo_hasredos());
	}

	return answer;
}

CROSSWORD* undo_redo(void)
{
	CROSSWORD* answer = 0;
	if (undo_buffer.undotop < undo_buffer.N -1)
	{
		answer = crossword_clone(undo_buffer.crosswords[undo_buffer.undotop +1]);
		undo_buffer.undotop++;
	}
	if (undo_buffer.enable)
	{
		(*undo_buffer.enable)(undo_buffer.ptr, undo_hasundos(), undo_hasredos());
	}

	return answer;
}

int undo_hasundos(void)
{
	return undo_buffer.undotop > 0 ? 1 : 0;
}

int undo_hasredos(void)
{
	return undo_buffer.undotop < undo_buffer.N -1 ? 1 : 0;
}
