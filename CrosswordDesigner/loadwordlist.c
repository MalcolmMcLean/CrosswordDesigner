#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "loadwordlist.h"

char **loadwordlist(const char *fname, int *N, int *err)
{
  FILE *fp;
  char buff[1024];
  char *word;
  char **answer = 0;
  int Nlines = 0;
  char **temp;
  int i, j;

  fp = fopen(fname, "r");
  if(!fp)
  {
    if(err)
	  *err = -2;
	return 0;
  }
  while(fgets(buff, 1024, fp))
  {
    temp = realloc(answer, (Nlines+1) * sizeof(char *));
	if(!temp)
	{
	  killwordlist(answer, Nlines);
	  if(err)
	    *err = -1;
	  return 0;
	}
	answer = temp;
	word = malloc(strlen(buff) + 1);
    if(!word)
	{
	  killwordlist(answer, Nlines);
	  if(err)
	    *err = -1;
	  return 0;

	}
	j = 0;
	for(i=0;buff[i];i++)
	  if(isalpha( (unsigned char) buff[i]))
	    word[j++] = toupper((unsigned char) buff[i]);
	word[j] = 0;
	if(j > 1)
	 answer[Nlines++] = word;
	else
	  free(word);
  }
  fclose(fp);
  *N = Nlines;
  return answer;
}

void shufflewordlist(char **list, int N)
{
  int i;
  int target;
  char *temp;

  for(i=0;i<N-1;i++)
  {
    target = (rand() % (N-i)) + i;
	temp = list[i];
	list[i] = list[target];
	list[target] = temp;
  }
}

void killwordlist(char **list, int N)
{
  int i;

  if(list)
  {
    for(i=0;i<N;i++)
	  free(list[i]);
	free(list);
  }
}

