#include <stdlib.h>
#include <string.h>

#include "wordmatcher.h"
#include "crossword.h"

#define uniform() (rand()/(RAND_MAX + 1.0))
#define rnd2(N) ( (int) (uniform() * uniform() * (N)) )

typedef struct
{
  char *constraint;
  char **list;
  int N;
  int Ntot;
  int id;
} CWWORD;

static int recursivefill(CROSSWORD *cw, CWWORD *words, int (*callback)(void *ptr), void *ptr);
static int finished(CWWORD *words, int N);
static int stuck(CWWORD *words, int N);
static int tryword(CROSSWORD *cw, CWWORD *words, int index);
static int untryword(CROSSWORD *cw, CWWORD *words, int index, char *constraint);
static int chooseword(CWWORD *words, int N);
static int updatecwword(CWWORD *words, int index);
static int downdatecwword(CWWORD *words, int index);
static int wordchanged(CROSSWORD *cw, CWWORD *word);
static void setword(CROSSWORD *cw, char *word, int id);

static CWWORD *getwordlist(CROSSWORD *cw);
static void killwordlist(CWWORD *words, int N);
static void getconstraint(CROSSWORD *cw, int id, char *ret);
static int wordintwice(CROSSWORD* cw, char* word);
static void killlist(char **list, int N);
static int domatch(char *word, char *wild);
static int strcount(char *str, int ch);
static char *mystrdup(char *str);

int fillgrid(CROSSWORD *cw, int (*callback)(void *ptr), void *ptr)
{
  CWWORD *words;
  int answer;
  char *original;
  int i, ii;

  original = malloc(cw->width * cw->height);
  if(!original)
    return -1;

  memcpy(original, cw->solution, cw->width * cw->height);

  do
  {
	for(i=0;i<cw->height;i++)
	  for(ii=0;ii<cw->width;ii++)
		  if(cw->grid[i*cw->width+ii] == 1)
	         crossword_setcell(cw, ii, i, original[i*cw->width+ii]);
    words = getwordlist(cw);
    answer = recursivefill(cw, words, callback, ptr);
    killwordlist(words, cw->Nacross + cw->Ndown);
  } while(answer == -1);

  free(original);
  return answer;
}

static int recursivefill(CROSSWORD *cw, CWWORD *words, int (*callback)(void *ptr), void *ptr)
{
	char *oldconstraint;
	int oldN;
    int index;
	int ans;

	if ( (*callback)(ptr))
		return 0;
	if(finished(words, cw->Nacross + cw->Ndown))
		return 0;
	if(stuck(words, cw->Nacross + cw->Ndown))
	   return -1;
    index = chooseword(words, cw->Nacross + cw->Ndown);
	oldconstraint = mystrdup(words[index].constraint);
	oldN = words[index].N;
	while(tryword(cw, words, index))
	{
	  ans = recursivefill(cw, words, callback, ptr);
	  if(ans == 0)
	  {
	    free(oldconstraint);
	    return 0;
	  }
	  words[index].N = --oldN;
	  untryword(cw, words, index, oldconstraint);
	}
    free(oldconstraint);
	return -1;
}

static int finished(CWWORD *words, int N)
{
  int i;

  for(i=0;i<N;i++)
    if(strcount(words[i].constraint, '?'))
		return 0;
  return 1;
}

static int stuck(CWWORD *words, int N)
{
  int i;

  for(i=0;i<N;i++)
    if(words[i].N == 0)
		return 1;
  return 0;
}

/*
  try a word. Select a possibility at random, then update constraints
    for all words that it crosses
  Params: cw - the puzzle
          words - the word list
		  index - index for word to fit
  Returns: 1 on success, 0 on no words left
  Note: N is maintained in caller
*/
static int tryword(CROSSWORD *cw, CWWORD *words, int index)
{
  int shot;
  char buff[256];
  char *temp;
  char *newword;
  int level;
  int i;

redo:

  if(words[index].N > 0)
  {
    shot = rnd2(words[index].N);
	temp = words[index].list[shot];
	words[index].list[shot] = words[index].list[words[index].N-1];
	words[index].list[words[index].N-1] = temp;
	words[index].N--;
	newword = words[index].list[words[index].N];
	setword(cw, newword, index);
  }
  else if(words[index].N == -1)
  {
    randword(buff, strlen(words[index].constraint), 0);
	newword = buff;
	setword(cw, newword, index);
  }
  else
    return 0;
  if(wordintwice(cw, newword))
    goto redo;
  strcpy(words[index].constraint, newword);
  words[index].N = -2;
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    if(wordchanged(cw, &words[i]))
	{
	  getconstraint(cw, words[i].id, buff);
	  if(!strchr(buff, '?'))
	  {
	    level = wordindictionary(buff);
		if(level == 0)
		  goto redo;
	  }
	}
  }

  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    if(wordchanged(cw, &words[i]))
	{
	  getconstraint(cw, words[i].id, words[i].constraint);
	  updatecwword(words, i);
	}
  }

  return 1;
}

/*
  backtracking. 
  The attempt to fit in a word has failed, so we need to make the constraints
    more generous again.
*/
static int untryword(CROSSWORD *cw, CWWORD *words, int index, char *constraint)
{
  int i;

  strcpy(words[index].constraint, constraint);
  setword(cw, constraint, index);
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    if(wordchanged(cw, &words[i]))
	{
	  getconstraint(cw, words[i].id, words[i].constraint);
	  downdatecwword(words, i);
	}
  }

  return 0;
}

/*
  choose the word to try next
*/
static int chooseword(CWWORD *words, int N)
{
  int i;
  int best = INT_MAX;
  int besti = -1;

  for(i=0;i<N;i++)
  {
    if(words[i].N > 0 && words[i].N < best)
	{
	  best = words[i].N;
	  besti = i;
	}
  }
  if(besti != -1)
    return besti;
  for(i=0;i<N;i++)
    if(words[i].N == -1)
	  return i;

  return -1;
}

/*
  update word (make constraints stricter)
  Params: words - the word list
          index - index of word to change
  Notes: matches the existing word list (to avoid trawling
    throught he entire databse again)
*/
static int updatecwword(CWWORD *words, int index)
{
  int i;
  char *temp;

  if(words[index].N > 0 && words[index].N < 200)
  {
    i = words[index].N;
    while(i--)
    {
		if(domatch(words[index].list[i], words[index].constraint) == 0)
		{
		  if(i < words[index].N-1)
		  {
		    temp = words[index].list[i];
			words[index].list[i] = words[index].list[words[index].N-1];
			words[index].list[words[index].N-1] = temp;
		  }
		  words[index].N--;
		}
    }
  }
  else
  {
    if(words[index].list && words[index].Ntot > 0)
      killlist(words[index].list, words[index].Ntot);
	words[index].list = matchword(words[index].constraint, 0, &words[index].N);
	words[index].Ntot = words[index].N;
  }
  
  return words[index].N;
}

/*
  undo updating (for backtracking)
  Params: words - the word list
          index - index of word to change
  Notes: whenthe constrints become more generous, we need
    to rebuild the list.
*/
static int downdatecwword(CWWORD *words, int index)
{
  
  if(strcount(words[index].constraint, '?') == strlen(words[index].constraint))
  {
	  killlist(words[index].list, words[index].Ntot);
	  words[index].list = 0;
	  words[index].Ntot = 0;
	  words[index].N = -1;
  }
  else
  {
     killlist(words[index].list, words[index].Ntot);
	 words[index].list = matchword(words[index].constraint, 0, &words[index].N);
	 words[index].Ntot = words[index].N;
  }

  return 0;
}

/*
  has a word's constraints changed as a result of the puzzle being
    altered?
  Params: cw - the puzzle
          word - word to test
*/
static int wordchanged(CROSSWORD *cw, CWWORD *word)
{
  char buff[256];

  if(strlen(word->constraint) > 255)
    exit(EXIT_FAILURE);

  getconstraint(cw, word->id, buff);
  if(strcmp(buff, word->constraint))
    return 1;
  return 0;
}

/*
  write the word to the puzzle
  Params: cw - the puzzle
          word - the word to write
		  id - id of word to write
  Notes: '?' will be written as a space character

*/
static void setword(CROSSWORD *cw, char *word, int id)
{
  int i, j;
  int ch;

  if(id < cw->Nacross)
  {
	  for(i=0;i<cw->width * cw->height;i++)
		  if(cw->numbers[i] == cw->numbersacross[id])
			  break;
	  for(j=0;word[j];j++)
	  {
		ch = word[j] == '?' ? ' ' : word[j];
	    crossword_setcell(cw, (i % cw->width) + j, i / cw->width, ch);
	  }
  }
  else
  {
	  for(i=0;i<cw->width * cw->height;i++)
		  if(cw->numbers[i] == cw->numbersdown[id-cw->Nacross])
			  break;
	  for(j=0;word[j];j++)
	  {
		ch = word[j] == '?' ? ' ' : word[j];
	    crossword_setcell(cw, (i % cw->width), i / cw->width + j, ch);
	  }
  }
}

/*
  generate the word list fromthe puzzle
  Parsms: cw - the puzzle
  Returns: word list
  Notes: words consist of constraints and lists of possibles.
    To avoid gobbling too much memory words with no
	constrints except length do not have lists. The number of
	possibilities is also limited to 100 per internal list
*/
static CWWORD *getwordlist(CROSSWORD *cw)
{
  CWWORD *answer;
  int len;
  int Nblank;
  int i;

  answer = malloc( (cw->Nacross + cw->Ndown) * sizeof(CWWORD));
  if(!answer)
    goto error_exit;
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    answer[i].constraint = 0;
    answer[i].list = 0;
	answer[i].N = 0;
	answer[i].Ntot = 0;
  }
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    answer[i].id = i;
	if(i < cw->Nacross)
		answer[i].constraint = malloc(strlen(cw->wordsacross[i]) + 1);
	else
		answer[i].constraint = malloc(strlen(cw->wordsdown[i-cw->Nacross]) + 1);
	if(!answer[i].constraint)
		goto error_exit;
	getconstraint(cw, i, answer[i].constraint);
  }
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    len = strlen(answer[i].constraint);
	Nblank = strcount(answer[i].constraint, '?');
	if(Nblank == 0)
	{
	  answer[i].N = -3;
	}
	else if(Nblank == len)
	{
	  answer[i].N = -1;
	}
	else
	{
	  answer[i].list = matchword(answer[i].constraint, 0, &answer[i].N);
	  answer[i].Ntot = answer[i].N;
	}
  }
  return answer;

error_exit:
  if(answer)
  {
	for(i=0;i<cw->Nacross + cw->Ndown;i++)
	{
      free(answer[i].constraint);
	  killlist(answer[i].list, answer[i].N);
	}
	free(answer);
  }
  return 0;
}

/*
  word list destructor
*/
static void killwordlist(CWWORD *words, int N)
{
  int i;

  if(words)
  {
    for(i=0;i<N;i++)
    {
	  killlist(words[i].list, words[i].Ntot);
	  free(words[i].constraint);
    }
	free(words);
  }

}


/*
  query the puzzle for a word's constraints
  Params: cw - the puzzle
          id - index of word
		  ret - return for constriants on that word. Set letters 
	  are leters, unset letters are '?'
*/
static void getconstraint(CROSSWORD *cw, int id, char *ret)
{
  int i;

	if(id < cw->Nacross)
	{
		for(i=0;cw->wordsacross[id][i];i++)
			ret[i] = cw->wordsacross[id][i] == ' ' ? '?' : cw->wordsacross[id][i];
	}
	else
	{
	  id -= cw->Nacross;
	  for(i=0;cw->wordsdown[id][i];i++)
			ret[i] = cw->wordsdown[id][i] == ' ' ? '?' : cw->wordsdown[id][i];
	}
	ret[i] = 0;
}

static int wordintwice(CROSSWORD *cw, char *word)
{
  int i;
  int Nfound = 0;

  for(i=0;i<cw->Nacross;i++)
	  if(!strcmp(cw->wordsacross[i], word))
	     Nfound++;

  for(i=0;i<cw->Ndown;i++)
	  if(!strcmp(cw->wordsdown[i], word))
	     Nfound++;

  if(Nfound > 1)
    return 1;
  else
    return 0;

}

/*
  string list destructor

*/
static void killlist(char **list, int N)
{
  int i;

  if(list)
  {
    for(i=0;i<N;i++)
      free(list[i]);
    free(list);
  }
}

/*
  does a word match a constraint (missing letters are represented by '?')
*/
static int domatch(char *word, char *wild)
{
   int i;

   for(i=0;word[i];i++)
     if(word[i] != wild[i] && wild[i] != '?')
	   return 0;

   return (wild[i] == 0) ? 1 : 0;
}

/*
  count the number of time ch occurs in str
*/
static int strcount(char *str, int ch)
{
  int answer = 0;

  while(*str)
    if(*str++ == ch)
		answer++;

  return answer;
}

/*
  strdup drop-in replacement
*/
static char *mystrdup(char *str)
{
  char *answer;

  answer = malloc(strlen(str) + 1);
  if(answer)
    strcpy(answer, str);

  return answer;
}
#if 0
#include <stdlib.h>
#include <string.h>

#include "wordmatcher.h"
#include "crossword.h"

#define uniform() (rand()/(RAND_MAX + 1.0))
#define rnd2(N) ( (int) (uniform() * uniform() * (N)) )

typedef struct
{
  char *constraint;
  char **list;
  int N;
  int Ntot;
  int id;
} CWWORD;

static int recursivefill(CROSSWORD *cw, CWWORD *words);
static int finished(CWWORD *words, int N);
static int stuck(CWWORD *words, int N);
static int tryword(CROSSWORD *cw, CWWORD *words, int index);
static int untryword(CROSSWORD *cw, CWWORD *words, int index, char *constraint);
static int chooseword(CWWORD *words, int N);
static int updatecwword(CWWORD *words, int index);
static int downdatecwword(CWWORD *words, int index);
static int wordchanged(CROSSWORD *cw, CWWORD *word);
static void setword(CROSSWORD *cw, char *word, int id);

static CWWORD *getwordlist(CROSSWORD *cw);
static void killwordlist(CWWORD *words, int N);
static void getconstraint(CROSSWORD *cw, int id, char *ret);
static void killlist(char **list, int N);
static int domatch(char *word, char *wild);
static int strcount(char *str, int ch);
static char *mystrdup(char *str);

int fillgrid(CROSSWORD *cw)
{
  CWWORD *words;
  int answer;

  words = getwordlist(cw);
  answer = recursivefill(cw, words);
  killwordlist(words, cw->Nacross + cw->Ndown);
  return answer;
}

static int recursivefill(CROSSWORD *cw, CWWORD *words)
{
	char *oldconstraint;
	int oldN;
    int index;
	int ans;

	if(finished(words, cw->Nacross + cw->Ndown))
		return 0;
	if(stuck(words, cw->Nacross + cw->Ndown))
	   return -1;
    index = chooseword(words, cw->Nacross + cw->Ndown);
	oldconstraint = mystrdup(words[index].constraint);
	oldN = words[index].N;
	while(tryword(cw, words, index))
	{
	  ans = recursivefill(cw, words);
	  if(ans == 0)
	  {
	    free(oldconstraint);
	    return 0;
	  }
	  words[index].N = --oldN;
	  untryword(cw, words, index, oldconstraint);
	}
    free(oldconstraint);
	return -1;
}

static int finished(CWWORD *words, int N)
{
  int i;

  for(i=0;i<N;i++)
    if(strcount(words[i].constraint, '?'))
		return 0;
  return 1;
}

static int stuck(CWWORD *words, int N)
{
  int i;

  for(i=0;i<N;i++)
    if(words[i].N == 0)
		return 1;
  return 0;
}

/*
  try a word. Select a possibility at random, then update constraints
    for all words that it crosses
  Params: cw - the puzzle
          words - the word list
		  index - index for word to fit
  Returns: 1 on success, 0 on no words left
  Note: N is maintained in caller
*/
static int tryword(CROSSWORD *cw, CWWORD *words, int index)
{
  int shot;
  char buff[256];
  char *temp;
  char *newword;
  int i;

  if(words[index].N > 0)
  {
    shot = rnd2(words[index].N);
	temp = words[index].list[shot];
	words[index].list[shot] = words[index].list[words[index].N-1];
	words[index].list[words[index].N-1] = temp;
	words[index].N--;
	newword = words[index].list[words[index].N];
	setword(cw, newword, index);
  }
  else if(words[index].N == -1)
  {
    randword(buff, strlen(words[index].constraint), 0);
	newword = buff;
	setword(cw, newword, index);
  }
  else
    return 0;
  strcpy(words[index].constraint, newword);
  words[index].N = -2;
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    if(wordchanged(cw, &words[i]))
	{
	  getconstraint(cw, words[i].id, words[i].constraint);
	  updatecwword(words, i);
	}
  }

  return 1;
}

/*
  backtracking. 
  The attempt to fit in a word has failed, so we need to make the constraints
    more generous again.
*/
static int untryword(CROSSWORD *cw, CWWORD *words, int index, char *constraint)
{
  int i;

  strcpy(words[index].constraint, constraint);
  setword(cw, constraint, index);
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    if(wordchanged(cw, &words[i]))
	{
	  getconstraint(cw, words[i].id, words[i].constraint);
	  downdatecwword(words, i);
	}
  }

  return 0;
}

/*
  choose the word to try next
*/
static int chooseword(CWWORD *words, int N)
{
  int i;
  int best = INT_MAX;
  int besti = -1;

  for(i=0;i<N;i++)
  {
    if(words[i].N > 0 && words[i].N < best)
	{
	  best = words[i].N;
	  besti = i;
	}
  }
  if(besti != -1)
    return besti;
  for(i=0;i<N;i++)
    if(words[i].N == -1)
	  return i;

  return -1;
}

/*
  update word (make constraints stricter)
  Params: words - the word list
          index - index of word to change
  Notes: matches the existing word list (to avoid trawling
    throught he entire databse again)
*/
static int updatecwword(CWWORD *words, int index)
{
  int i;
  char *temp;

  if(words[index].N > 0 && words[index].N < 200)
  {
    i = words[index].N;
    while(i--)
    {
		if(domatch(words[index].list[i], words[index].constraint) == 0)
		{
		  if(i < words[index].N-1)
		  {
		    temp = words[index].list[i];
			words[index].list[i] = words[index].list[words[index].N-1];
			words[index].list[words[index].N-1] = temp;
		  }
		  words[index].N--;
		}
    }
  }
  else
  {
    if(words[index].list && words[index].Ntot > 0)
      killlist(words[index].list, words[index].Ntot);
	words[index].list = matchword(words[index].constraint, 2, &words[index].N);
	words[index].Ntot = words[index].N;
  }
  
  return words[index].N;
}

/*
  undo updating (for backtracking)
  Params: words - the word list
          index - index of word to change
  Notes: whenthe constrints become more generous, we need
    to rebuild the list.
*/
static int downdatecwword(CWWORD *words, int index)
{
  
  if(strcount(words[index].constraint, '?') == strlen(words[index].constraint))
  {
	  killlist(words[index].list, words[index].Ntot);
	  words[index].list = 0;
	  words[index].Ntot = 0;
	  words[index].N = -1;
  }
  else
  {
     killlist(words[index].list, words[index].Ntot);
	 words[index].list = matchword(words[index].constraint, 2, &words[index].N);
	 words[index].Ntot = words[index].N;
  }

  return 0;
}

/*
  has a word's constraints changed as a result of the puzzle being
    altered?
  Params: cw - the puzzle
          word - word to test
*/
static int wordchanged(CROSSWORD *cw, CWWORD *word)
{
  char buff[256];

  if(strlen(word->constraint) > 255)
    exit(EXIT_FAILURE);

  getconstraint(cw, word->id, buff);
  if(strcmp(buff, word->constraint))
    return 1;
  return 0;
}

/*
  write the word to the puzzle
  Params: cw - the puzzle
          word - the word to write
		  id - id of word to write
  Notes: '?' will be written as a space character

*/
static void setword(CROSSWORD *cw, char *word, int id)
{
  int i, j;
  int ch;

  if(id < cw->Nacross)
  {
	  for(i=0;i<cw->width * cw->height;i++)
		  if(cw->numbers[i] == cw->numbersacross[id])
			  break;
	  for(j=0;word[j];j++)
	  {
		ch = word[j] == '?' ? ' ' : word[j];
	    crossword_setcell(cw, (i % cw->width) + j, i / cw->width, ch);
	  }
  }
  else
  {
	  for(i=0;i<cw->width * cw->height;i++)
		  if(cw->numbers[i] == cw->numbersdown[id-cw->Nacross])
			  break;
	  for(j=0;word[j];j++)
	  {
		ch = word[j] == '?' ? ' ' : word[j];
	    crossword_setcell(cw, (i % cw->width), i / cw->width + j, ch);
	  }
  }
}

/*
  generate the word list fromthe puzzle
  Parsms: cw - the puzzle
  Returns: word list
  Notes: words consist of constraints and lists of possibles.
    To avoid gobbling too much memory words with no
	constrints except length do not have lists. The number of
	possibilities is also limited to 100 per internal list
*/
static CWWORD *getwordlist(CROSSWORD *cw)
{
  CWWORD *answer;
  int len;
  int Nblank;
  int i;

  answer = malloc( (cw->Nacross + cw->Ndown) * sizeof(CWWORD));
  if(!answer)
    goto error_exit;
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    answer[i].constraint = 0;
    answer[i].list = 0;
	answer[i].N = 0;
	answer[i].Ntot = 0;
  }
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    answer[i].id = i;
	if(i < cw->Nacross)
		answer[i].constraint = malloc(strlen(cw->wordsacross[i]) + 1);
	else
		answer[i].constraint = malloc(strlen(cw->wordsdown[i-cw->Nacross]) + 1);
	if(!answer[i].constraint)
		goto error_exit;
	getconstraint(cw, i, answer[i].constraint);
  }
  for(i=0;i<cw->Nacross + cw->Ndown;i++)
  {
    len = strlen(answer[i].constraint);
	Nblank = strcount(answer[i].constraint, '?');
	if(Nblank == 0)
	{
	  answer[i].N = -3;
	}
	else if(Nblank == len)
	{
	  answer[i].N = -1;
	}
	else
	{
	  answer[i].list = matchword(answer[i].constraint, 2, &answer[i].N);
	  answer[i].Ntot = answer[i].N;
	}
  }
  return answer;

error_exit:
  if(answer)
  {
	for(i=0;i<cw->Nacross + cw->Ndown;i++)
	{
      free(answer[i].constraint);
	  killlist(answer[i].list, answer[i].N);
	}
	free(answer);
  }
  return 0;
}

/*
  word list destructor
*/
static void killwordlist(CWWORD *words, int N)
{
  int i;

  if(words)
  {
    for(i=0;i<N;i++)
    {
	  killlist(words[i].list, words[i].Ntot);
	  free(words[i].constraint);
    }
	free(words);
  }

}


/*
  query the puzzle for a word's constraints
  Params: cw - the puzzle
          id - index of word
		  ret - return for constriants on that word. Set letters 
	  are leters, unset letters are '?'
*/
static void getconstraint(CROSSWORD *cw, int id, char *ret)
{
  int i;

	if(id < cw->Nacross)
	{
		for(i=0;cw->wordsacross[id][i];i++)
			ret[i] = cw->wordsacross[id][i] == ' ' ? '?' : cw->wordsacross[id][i];
	}
	else
	{
	  id -= cw->Nacross;
	  for(i=0;cw->wordsdown[id][i];i++)
			ret[i] = cw->wordsdown[id][i] == ' ' ? '?' : cw->wordsdown[id][i];
	}
	ret[i] = 0;
}

/*
  string list destructor

*/
static void killlist(char **list, int N)
{
  int i;

  if(list)
  {
    for(i=0;i<N;i++)
      free(list[i]);
    free(list);
  }
}

/*
  does a word match a constraint (missing letters are represented by '?')
*/
static int domatch(char *word, char *wild)
{
   int i;

   for(i=0;word[i];i++)
     if(word[i] != wild[i] && wild[i] != '?')
	   return 0;

   return (wild[i] == 0) ? 1 : 0;
}

/*
  count the number of time ch occurs in str
*/
static int strcount(char *str, int ch)
{
  int answer = 0;

  while(*str)
    if(*str++ == ch)
		answer++;

  return answer;
}

/*
  strdup drop-in replacement
*/
static char *mystrdup(char *str)
{
  char *answer;

  answer = malloc(strlen(str) + 1);
  if(answer)
    strcpy(answer, str);

  return answer;
}



#endif


