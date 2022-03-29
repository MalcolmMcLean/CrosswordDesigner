#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

#include "wordmatcher.h"

extern char *english_words_10[4393];
extern char *english_words_20[7951];
extern char *english_words_35[36381];
extern char *english_words_40[6929];
extern char *english_words_50[31118];
extern char *english_words_55[6311];
extern char *english_words_60[13508];
extern char *english_words_70[39825];
extern char* english_words_85[145323];
extern char* english_words_95[221726];

extern char *english_phrases[7289];

static uint64_t* reg_wordsbylength[8] = {0,};
static int Nwordsbylength[8] = {0,};
static int wordsbylengthinit = 0;

static uint64_t* reg_english_words_10 = 0;
static uint64_t* reg_english_words_20 = 0;
static uint64_t* reg_english_words_35 = 0;
static uint64_t* reg_english_words_40 = 0;
static uint64_t* reg_english_words_50 = 0;
static uint64_t* reg_english_words_55 = 0;
static uint64_t* reg_english_words_60 = 0;
static uint64_t *reg_english_words_70 = 0;
static uint64_t* reg_english_words_85 = 0;
static uint64_t* reg_english_words_95 = 0;

static uint64_t* reg_english_phrases = 0;


static char **matchlists(char **list, int N, char *wild, int Nmax, int *Nret);
static char **anagramlists(char **list, int N, char *word, int Nmax, int *Nret);
static char **catN(int N, ...);
static char **catlist(char **a, int Na, char **b, int Nb);
static int domatch(char *word, char *wild);
static int isanagram(char *str1, char *str2);
static char** matchregfast(uint64_t* reg, int N, char* wild, int Nmax, int* Nret);
static char** matchlistsfast(char** list, uint64_t* reg, int N, char* wild, int Nmax, int* Nret);
static uint64_t* registerfromlist(const char** list, int N);
static uint64_t maskfromstring(const char* string);
static uint64_t registerfromstring(const char* string);

static void killlist(char **list, int N);
static int wordinlist(char **list, int N, char *word);
static int strcount(char *str, int ch);
static char *mystrdup(const char *str);

int wordmatcherinit(void)
{
	int len;

	for (len = 2; len < 8; len++)
	{
		char** lenlist = getwordsoflength(len, &Nwordsbylength[len]);
		if (!lenlist)
			goto error_exit;
		reg_wordsbylength[len] = registerfromlist(lenlist, Nwordsbylength[len]);
		killlist(lenlist, Nwordsbylength[len]);
		if (!reg_wordsbylength[len])
			goto error_exit;
	}

	reg_english_words_10 = registerfromlist(english_words_10, 4393);
	reg_english_words_20 = registerfromlist(english_words_20, 7951);
	reg_english_words_35 = registerfromlist(english_words_35, 36381);
	reg_english_words_40 = registerfromlist(english_words_40, 6929);
	reg_english_words_50 = registerfromlist(english_words_50, 31118);
	reg_english_words_55 = registerfromlist(english_words_55, 6311);
	reg_english_words_60 = registerfromlist(english_words_60, 13508);
	reg_english_words_70 = registerfromlist(english_words_70, 39825);
	reg_english_words_85 = registerfromlist(english_words_85, 145323);
	reg_english_words_95 = registerfromlist(english_words_95, 221726);
	reg_english_phrases = registerfromlist(english_phrases, 7289);

	if (!reg_english_words_10)
		goto error_exit;
	if (!reg_english_words_20)
		goto error_exit;
	if (!reg_english_words_35)
		goto error_exit;
	if (!reg_english_words_40)
		goto error_exit;
	if (!reg_english_words_50)
		goto error_exit;
	if (!reg_english_words_55)
		goto error_exit;
	if (!reg_english_words_60)
		goto error_exit;
	if (!reg_english_words_70)
		goto error_exit;
	if (!reg_english_words_85)
		goto error_exit;
	if (!reg_english_words_95)
		goto error_exit;
	if (!reg_english_phrases)
		goto error_exit;
	wordsbylengthinit = 1;

	return 0;
error_exit:
	for (len = 2; len < 8; len++)
	{
		free(reg_wordsbylength[len]);
		reg_wordsbylength[len] = 0;
		Nwordsbylength[len] = 0;
	}
		
	free(reg_english_words_10);
	reg_english_words_10 = 0;
	free(reg_english_words_20);
	reg_english_words_20 = 0;
	free(reg_english_words_35);
	reg_english_words_35 = 0;
	free(reg_english_words_40);
	reg_english_words_40 = 0;
	free(reg_english_words_50);
	reg_english_words_50 = 0;
	free(reg_english_words_55);
	reg_english_words_55 = 0;
	free(reg_english_words_60);
	reg_english_words_60 = 0;
	free(reg_english_words_70);
	reg_english_words_70 = 0;
	free(reg_english_words_85);
	reg_english_words_85 = 0;
	free(reg_english_words_95);
	reg_english_words_95 = 0;
	free(reg_english_phrases);
	reg_english_phrases = 0;
	
	return -1;
}

char** getwordsoflength(int len, int* N)
{
	char wild[256];
	int i;
	for (i = 0; i < len; i++)
		wild[i] = '?';
	wild[i] = 0;

	char** list_10;
	char** list_20;
	char** list_35;
	char** list_40;
	char** list_50;
	char** list_55;
	char** list_60;
	char** list_70;
	char** list_85;
	char** list_95;
	char** list_phrases;
	int N10;
	int N20;
	int N35;
	int N40;
	int N50;
	int N55;
	int N60;
	int N70;
	int N85;
	int N95;
	int Nphrases;

	char** answer;

	list_10 = matchlists(english_words_10, 4393, wild, INT_MAX, &N10);
	list_20 = matchlists(english_words_20, 7951, wild, INT_MAX, &N20);
	list_35 = matchlists(english_words_35, 36381, wild, INT_MAX, &N35);
	list_40 = matchlists(english_words_40, 6929, wild, INT_MAX, &N40);
	list_50 = matchlists(english_words_50, 31118, wild, INT_MAX, &N50);
	list_55 = matchlists(english_words_55, 6311, wild, INT_MAX, &N55);
	list_60 = matchlists(english_words_60, 13508, wild, INT_MAX, &N60);
	list_70 = matchlists(english_words_70, 39825, wild, INT_MAX, &N70);
	list_85 = matchlists(english_words_85, 145323, wild, INT_MAX, &N85);
	list_95 = matchlists(english_words_95, 221726, wild, INT_MAX, &N95);
	list_phrases = matchlists(english_phrases, 7289, wild, INT_MAX, &Nphrases);

	answer = catN(11, list_10, N10, list_20, N20, list_35, N35,
		list_40, N40, list_50, N50, list_55, N55,
		list_60, N60, list_70, N70, list_85, N85, list_95, N95,
		list_phrases, Nphrases);
	if (!answer)
		return 0;
	*N = N10 + N20 + N35 + N40 + N50 + N55 + N60 + N70 + N85 + N95 + Nphrases;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_40);
	free(list_50);
	free(list_55);
	free(list_60);
	free(list_70);
	free(list_85);
	free(list_95);
	free(list_phrases);

	return answer;
}


char **matchword(char *word, int level, int *N)
{
  char **list_10;
  char **list_20;
  char **list_35;
  char **list_40;
  char **list_50;
  char **list_55;
  char **list_60;
  char **list_70;
  char** list_85;
  char** list_95;
  char **list_phrases;
  int N10;
  int N20;
  int N35;
  int N40;
  int N50;
  int N55;
  int N60;
  int N70;
  int N85;
  int N95;
  int Nphrases;
  char **answer;

  *N = 0;

  if (level == 3)
  {
	  int len = strlen(word);
	  if (len >= 2 && len < 8)
	  {
		  return matchregfast(reg_wordsbylength[len], Nwordsbylength[len], word, 800, N);
	  }
  }

  

  switch(level)
  {
  case 0:
    list_10 = matchlists(english_words_10, 4393, word, 100, &N10);
	list_20 = matchlists(english_words_20, 7951, word, 100, &N20);
	list_35 = matchlists(english_words_35, 36381, word, 100, &N35);
	list_phrases = matchlists(english_phrases, 7289, word, 100, &Nphrases);
	answer = catN(4, list_10, N10, list_20, N20, list_35, N35, list_phrases, Nphrases);
	if(!answer)
	   return 0;
	*N = N10 + N20 + N35 + Nphrases;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_phrases);
	return answer;
  case 1:
	list_10 = matchlists(english_words_10, 4393, word, 100, &N10);
	list_20 = matchlists(english_words_20, 7951, word, 100, &N20);
	list_35 = matchlists(english_words_35, 36381, word, 100, &N35);
	list_40 = matchlists(english_words_40, 6929, word, 100, &N40);
    list_50 = matchlists(english_words_50, 31118, word, 100, &N50);
    list_55 = matchlists(english_words_55, 6311, word, 100, &N55); 
	answer = catN(6, list_10, N10, list_20, N20, list_35, N35, list_40, N40, list_50, N50, list_55, N55);
	if(!answer)
	  return 0;
	*N = N10 + N20 + N35 + N40 + N50 + N55;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_40);
	free(list_50);
	free(list_55);
	return answer;
    
  case 2:
	list_10 = matchlists(english_words_10, 4393, word, 100, &N10);
	list_20 = matchlists(english_words_20, 7951, word, 100, &N20);
	list_35 = matchlists(english_words_35, 36381, word, 100, &N35);
	list_40 = matchlists(english_words_40, 6929, word, 100, &N40);
    list_50 = matchlists(english_words_50, 31118, word, 100, &N50);
    list_55 = matchlists(english_words_55, 6311, word, 100, &N55); 
	list_60 = matchlists(english_words_60, 13508, word, 100, &N60);
	list_70 = matchlists(english_words_70, 39825, word, 100, &N70);
	answer = catN(8, list_10, N10, list_20, N20, list_35, N35, 
		list_40, N40, list_50, N50, list_55, N55,
		list_60, N60, list_70, N70);
	if(!answer)
	  return 0;
	*N = N10 + N20 + N35 + N40 + N50 + N55 + N60 + N70;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_40);
	free(list_50);
	free(list_55);
	free(list_60);
	free(list_70);
	return answer;
  case 3:
	  list_10 = matchlistsfast(english_words_10, reg_english_words_10, 4393, word, 100, &N10);
	  list_20 = matchlistsfast(english_words_20, reg_english_words_20, 7951, word, 100, &N20);
	  list_35 = matchlistsfast(english_words_35, reg_english_words_35, 36381, word, 100, &N35);
	  list_40 = matchlistsfast(english_words_40, reg_english_words_40, 6929, word, 100, &N40);
	  list_50 = matchlistsfast(english_words_50, reg_english_words_50, 31118, word, 100, &N50);
	  list_55 = matchlistsfast(english_words_55, reg_english_words_55, 6311, word, 100, &N55);
	  list_60 = matchlistsfast(english_words_60, reg_english_words_60, 13508, word, 100, &N60);
	  list_70 = matchlistsfast(english_words_70, reg_english_words_70, 39825, word, 100, &N70);
	  list_85 = matchlistsfast(english_words_85, reg_english_words_85, 145323, word, 100, &N85);
	  list_95 = matchlistsfast(english_words_95, reg_english_words_95, 221726, word, 100, &N95);
	  answer = catN(10, list_10, N10, list_20, N20, list_35, N35,
		  list_40, N40, list_50, N50, list_55, N55,
		  list_60, N60, list_70, N70, list_85, N85, list_95, N95);
	  if (!answer)
		  return 0;
	  *N = N10 + N20 + N35 + N40 + N50 + N55 + N60 + N70 + N85 + N95;
	  free(list_10);
	  free(list_20);
	  free(list_35);
	  free(list_40);
	  free(list_50);
	  free(list_55);
	  free(list_60);
	  free(list_70);
	  free(list_85);
	  free(list_95);
	  return answer;
  default:
    assert(level >= 0 && level < 4);
	return 0;
  }
  return 0;
}

char **findanagrams(char *word, int level, int *N)
{
  char **list_10;
  char **list_20;
  char **list_35;
  char **list_40;
  char **list_50;
  char **list_55;
  char **list_60;
  char **list_70;
  char** list_85;
  char** list_95;
  int N10;
  int N20;
  int N35;
  int N40;
  int N50;
  int N55;
  int N60;
  int N70;
  int N85;
  int N95;
  char **answer;

  *N = 0;

  switch(level)
  {
  case 0:
    list_10 = anagramlists(english_words_10, 4393, word, 100, &N10);
	list_20 = anagramlists(english_words_20, 7951, word, 100, &N20);
	list_35 = anagramlists(english_words_35, 36381, word, 100, &N35);
	answer = catN(3, list_10, N10, list_20, N20, list_35, N35);
	if(!answer)
	   return 0;
	*N = N10 + N20 + N35;
	free(list_10);
	free(list_20);
	free(list_35);
	return answer;
  case 1:
	list_10 = anagramlists(english_words_10, 4393, word, 100, &N10);
	list_20 = anagramlists(english_words_20, 7951, word, 100, &N20);
	list_35 = anagramlists(english_words_35, 36381, word, 100, &N35);
	list_40 = anagramlists(english_words_40, 6929, word, 100, &N40);
    list_50 = anagramlists(english_words_50, 31118, word, 100, &N50);
    list_55 = anagramlists(english_words_55, 6311, word, 100, &N55); 
	answer = catN(6, list_10, N10, list_20, N20, list_35, N35, list_40, N40, list_50, N50, list_55, N55);
	if(!answer)
	  return 0;
	*N = N10 + N20 + N35 + N40 + N50 + N55;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_40);
	free(list_50);
	free(list_55);
	return answer;
    
  case 2:
	list_10 = anagramlists(english_words_10, 4393, word, 100, &N10);
	list_20 = anagramlists(english_words_20, 7951, word, 100, &N20);
	list_35 = anagramlists(english_words_35, 36381, word, 100, &N35);
	list_40 = anagramlists(english_words_40, 6929, word, 100, &N40);
    list_50 = anagramlists(english_words_50, 31118, word, 100, &N50);
    list_55 = anagramlists(english_words_55, 6311, word, 100, &N55); 
	list_60 = anagramlists(english_words_60, 13508, word, 100, &N60);
	list_70 = anagramlists(english_words_70, 39825, word, 100, &N70);
	answer = catN(8, list_10, N10, list_20, N20, list_35, N35, 
		list_40, N40, list_50, N50, list_55, N55,
		list_60, N60, list_70, N70);
	if(!answer)
	  return 0;
	*N = N10 + N20 + N35 + N40 + N50 + N55 + N60 + N70;
	free(list_10);
	free(list_20);
	free(list_35);
	free(list_40);
	free(list_50);
	free(list_55);
	free(list_60);
	free(list_70);
	return answer;
  case 3:
	  list_10 = anagramlists(english_words_10, 4393, word, 100, &N10);
	  list_20 = anagramlists(english_words_20, 7951, word, 100, &N20);
	  list_35 = anagramlists(english_words_35, 36381, word, 100, &N35);
	  list_40 = anagramlists(english_words_40, 6929, word, 100, &N40);
	  list_50 = anagramlists(english_words_50, 31118, word, 100, &N50);
	  list_55 = anagramlists(english_words_55, 6311, word, 100, &N55);
	  list_60 = anagramlists(english_words_60, 13508, word, 100, &N60);
	  list_70 = anagramlists(english_words_70, 39825, word, 100, &N70);
	  list_85 = anagramlists(english_words_85, 143323, word, 100, &N85);
	  list_95 = anagramlists(english_words_95, 221726, word, 100, &N95);
	  answer = catN(10, list_10, N10, list_20, N20, list_35, N35,
		  list_40, N40, list_50, N50, list_55, N55,
		  list_60, N60, list_70, N70, list_85, N85, list_95, N95);
	  if (!answer)
		  return 0;
	  *N = N10 + N20 + N35 + N40 + N50 + N55 + N60 + N70 + N85 * N95;
	  free(list_10);
	  free(list_20);
	  free(list_35);
	  free(list_40);
	  free(list_50);
	  free(list_55);
	  free(list_60);
	  free(list_70);
	  free(list_85);
	  free(list_95);
	  return answer;
  default:
    assert(level >= 0 && level < 4);
	return 0;
  }
  return 0;
}

int randword(char *ret, int len, int level)
{
  int i;
  int breaker = 0;

  do
  {
    i = rand() % 4393;
    while(i < 4393 && strlen(english_words_10[i]) != len)
		i++;
	if(i != 4393 && !strchr(english_words_10[i], '\''))
	{
	  strcpy(ret, english_words_10[i]);
	  return 0;
	}
  }
  while(breaker++ < 100);

  return -1;
}

int wordindictionary(char *word)
{
  if(wordinlist(english_words_10, 4393, word))
    return 10;
  if(wordinlist(english_words_20, 7951, word))
    return 20;
  if(wordinlist(english_words_35, 36381, word))
    return 35;
  if(wordinlist(english_words_40, 6929, word))
    return 40;
  if(wordinlist(english_words_50, 31118, word))
    return 50;
  if(wordinlist(english_words_55, 6311, word))
    return 55;
  if(wordinlist(english_words_60, 13508, word))
    return 60;
  if(wordinlist(english_words_70, 39825, word))
    return 70;
  if (wordinlist(english_words_85, 145323, word))
	  return 85;
  if (wordinlist(english_words_95, 221726, word))
	  return 95; 

  return 0;
}

static char **matchlists(char **list, int N, char *wild, int Nmax, int *Nret)
{
  char **answer = 0;
  int buffsize = 0;
  int Nfound = 0;
  void *temp;
  int i;

  for(i=0;i<N;i++)
  {
    if(domatch(list[i], wild))
	{
	  if(Nfound == buffsize)
	  {
	    buffsize = buffsize * 2 + 10;
	    temp = realloc(answer, buffsize * sizeof(char *));
		if(!temp)
		  goto error_exit;
		answer = temp;
	  }
      answer[Nfound] = mystrdup(list[i]);
	  if(!answer[Nfound])
		  goto error_exit;
	  Nfound++;
	  if(Nfound == Nmax)
	    break;
	}
  }
  *Nret = Nfound;
  return answer;

error_exit:
  for(i=0;i<Nfound;i++)
    free(answer[i]);
  free(answer);
  *Nret = -1;
  return 0;
}

static char **anagramlists(char **list, int N, char *word, int Nmax, int *Nret)
{
  char **answer = 0;
  int buffsize = 0;
  int Nfound = 0;
  void *temp;
  int i;

  for(i=0;i<N;i++)
  {
    if(isanagram(list[i], word))
	{
	  if(Nfound == buffsize)
	  {
	    buffsize = buffsize * 2 + 10;
	    temp = realloc(answer, buffsize * sizeof(char *));
		if(!temp)
		  goto error_exit;
		answer = temp;
	  }
      answer[Nfound] = mystrdup(list[i]);
	  if(!answer[Nfound])
		  goto error_exit;
	  Nfound++;
	  if(Nfound == Nmax)
	    break;
	}
  }
  *Nret = Nfound;
  return answer;

error_exit:
  for(i=0;i<Nfound;i++)
    free(answer[i]);
  free(answer);
  *Nret = -1;
  return 0;
}

static char **catN(int N, ...)
{
  va_list vargs;
  char **answer = 0;
  char **temp;
  char **list;
  int Ntot = 0;
  int Nl;
  int i;

  va_start(vargs, N);
  for(i=0;i<N;i++)
  {
    list = va_arg(vargs, char **);
	Nl = va_arg(vargs, int);
	temp = catlist(answer, Ntot, list, Nl);
	free(answer);
	Ntot += Nl;
	answer = temp;
  }
  va_end(vargs);

  return answer;
}

static char **catlist(char **a, int Na, char **b, int Nb)
{
  char **answer;
  int i;

  answer = malloc((Na + Nb) * sizeof(char *));
  if(!answer)
	  return 0;
  for(i=0;i<Na;i++)
    answer[i] = a[i];
  for(i=0;i<Nb;i++)
    answer[i+Na] = b[i];

  return answer;
}


static int domatch(char *word, char *wild)
{
   int i;

   for(i=0;word[i];i++)
   {
     if(word[i] != wild[i] && wild[i] != '?')
	   return 0;
     if(word[i] == '\'')
	   return 0;
   }

   return (wild[i] == 0) ? 1 : 0;
}

static int isanagram(char *str1, char *str2)
{
  int i;

  if(strlen(str1) != strlen(str2))
    return 0;
  for(i=0;str1[i];i++)
    if(strcount(str1, str1[i]) != strcount(str2, str1[i]))
	  return 0;
  return 1;
}

static char* regtostring(uint64_t reg)
{
	uint64_t mask = 0xFF00000000000000;
	char* answer = malloc(8);
	int i;

	assert((reg & 0xFF) == 0);
	assert(answer);
	if (!answer)
		return 0;
	for (i = 0; i < 8; i++)
	{
		answer[i] = (char)((reg & mask) >> 56);
		reg <<= 8;
	}
	assert(answer[7] == 0);

	return answer;
}

#include <inttypes.h>

static char** matchregfast(uint64_t* reg, int N, char* wild, int Nmax, int *Nret)
{
	char** answer = 0;
	int buffsize = 0;
	int Nfound = 0;
	void* temp;
	uint64_t mask = maskfromstring(wild);
	uint64_t wildreg = registerfromstring(wild);
	uint64_t wildregmasked = wildreg & mask;
	int i;

	int lenwild = strlen(wild);
	assert(lenwild > 1 && lenwild < 8);

	for (i = 0; i < N; i++)
	{
		if ((reg[i] & mask) == wildregmasked)
		{
			if (Nfound == buffsize)
			{
				buffsize = buffsize * 2 + 10;
				temp = realloc(answer, buffsize * sizeof(char*));
				if (!temp)
					goto error_exit;
				answer = temp;
			}
			answer[Nfound] = regtostring(reg[i]);
			assert(strlen(answer[Nfound]) == lenwild);
			if (!answer[Nfound])
				goto error_exit;
			Nfound++;
			if (Nfound == Nmax)
				break;
		}
	}

	*Nret = Nfound;
	return answer;

error_exit:
	assert(0);
	for (i = 0; i < Nfound; i++)
		free(answer[i]);
	free(answer);
	*Nret = -1;
	return 0;


}


static char** matchlistsfast(char** list, uint64_t *reg, int N, char* wild, int Nmax, int* Nret)
{
	char** answer = 0;
	int buffsize = 0;
	int Nfound = 0;
	void* temp;
	int len = strlen(wild);
	uint64_t mask = maskfromstring(wild);
	uint64_t wildreg = registerfromstring(wild);
	uint64_t wildregmasked = wildreg & mask;
	uint64_t wildregwilds = wildreg & ~mask;
	uint64_t questions = 0x3F3F3F3F3F3F3F3F;
	uint64_t wilds = questions & ~mask;
	int i;
	

	for (i = 0; i < N; i++)
	{
		if ((reg[i] & mask) == wildregmasked)
		{
			if ( (len < 8 && ((reg[i] & mask) | wilds) == wildreg) || domatch(list[i], wild))
			{
				if (Nfound == buffsize)
				{
					buffsize = buffsize * 2 + 10;
					temp = realloc(answer, buffsize * sizeof(char*));
					if (!temp)
						goto error_exit;
					answer = temp;
				}
				answer[Nfound] = mystrdup(list[i]);
				if (!answer[Nfound])
					goto error_exit;
				Nfound++;
				if (Nfound == Nmax)
					break;
			}
		}
	}
	*Nret = Nfound;
	return answer;

error_exit:
	for (i = 0; i < Nfound; i++)
		free(answer[i]);
	free(answer);
	*Nret = -1;
	return 0;


}

static uint64_t* registerfromlist(const char** list, int N)
{
	int i;
	uint64_t* answer = malloc(N * sizeof(uint64_t));
	if (!answer)
		return 0;
	for (i = 0; i < N; i++)
		answer[i] = registerfromstring(list[i]);

	return answer;
}

static uint64_t maskfromstring(const char* string)
{
	uint64_t answer = 0;
	int len = strlen(string);
	int i;
	for (i = 0; i < 8; i++)
	{
		answer <<= 8;
		if (i >= len || (string[i] != '?') )
			answer |= 0xFF;
	}

	return answer;
}

static uint64_t registerfromstring(const char* string)
{
	uint64_t answer = 0;
	int len = strlen(string);
	int i;
	for (i = 0; i < 8; i++)
	{
		answer <<= 8;
		if (i < len)
			answer |= string[i];

	}

	return answer;
}

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

static int wordinlist(char **list, int N, char *word)
{
  int low = 0;
  int high = N-1;
  int mid;
  int cmp;

  while(low <= high)
  {
    mid = (low + high) /2;
	cmp = strcmp(list[mid], word);
	if(cmp == 0)
	  return 1;
	if(cmp < 0)
	  low = mid+1;
	else
	  high = mid-1;
  }

  return 0;

}

static int strcount(char *str, int ch)
{
  int answer = 0;
  while(*str)
    if(*str++ == ch)
	  answer++;
  return answer;
}

static char *mystrdup(const char *str)
{
  char *answer;

  answer = malloc(strlen(str)+1);
  if(answer)
    strcpy(answer, str);
  return answer;
}