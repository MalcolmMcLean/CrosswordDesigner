#include "blankgrids.h"

#include <stdlib.h>
#include <string.h>

extern BLANKGRID americanblankgrids[4583];

static char* mystrdup(const char* str);

char *randomamericanblankgrid(int *width, int *height)
{
	char* answer = 0;
	int index;
	do
	{
		index = rand() % 4583;
	} while (americanblankgrids[index].width != 15 || americanblankgrids[index].height != 15);

	answer = mystrdup(americanblankgrids[index].grid);
	if (width)
		*width = americanblankgrids[index].width;
	if (height)
		*height = americanblankgrids[index].height;

	return answer;
}

static char* mystrdup(const char* str)
{
	char* answer = malloc(strlen(str) + 1);
	if (answer)
		strcpy(answer, str);
	return answer;
}