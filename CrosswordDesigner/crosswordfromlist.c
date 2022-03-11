#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "anneal.h"
#include "crossword.h"

#define ACROSS 0
#define DOWN 1



typedef struct
{
  char **words;
  int N;
  int pad;
} SOLUTION;

typedef struct
{
  char *puzzle;
  int width;
  int height;
  int quiet;
} WORKSPACE;

typedef struct
{
  int word;
  int x;
  int y;
  int dir;
} ENTRY;

static void *clone(void *obj, void *ptr);
static void kill(void *obj, void *ptr);
static int copy(void *dest, void *src, void *ptr);
static void mutate(void *obj, void *ptr);
static double score(void *obj, void *ptr);

static ENTRY *greedy(char *puzzle, int width, int height, char **words, int N, int pad, int *Nused);
static int placeword(char *puzzle, int width, int height, char *word, int *x, int *y, int *dir);


/*
  function to generate a crossword (heart of the program)
  Params: puzzle - return buffer for crossword. Blanks are represented with zeroes
          width - puzzle width
		  height - puzzle height
		  dict - dictionary of allowed words (make all upper case)
		  N - number of allowed words
  We shuffle the wortd list, and perform greedy best matching. Then we use
  simulated annealing with an order exchange as the mutation function, to get
  a good crossword that incorporates as many words as possible
*/
void crosswordfromlist(char *puzzle, int width, int height, char **dict, int N, int iter,int quiet)
{
  WORKSPACE workspace;
  SOLUTION solution;
  int i;

  workspace.puzzle = puzzle;
  workspace.height = height;
  workspace.width = width;
  workspace.quiet = quiet;

  solution.N = N;
  solution.pad = 0;
  solution.words = malloc(N * sizeof(char *));
  for(i=0;i<N;i++)
    solution.words[i] = dict[i];

  anneal(&solution, clone, kill, copy, score, mutate, iter, &workspace);
  score(&solution, &workspace);

  free(solution.words);
}

/*
  generate a list of square numbers for a puzzle
  Params: puzzle - the puzzle. Filled squares must return true for isalpha()
          width - puzzle width
		  height - puzzle height
  Returns list of numbers (most will be zero)
*/
int *generatenumbers(char *puzzle, int width, int height)
{
  int *answer;
  int north;
  int south;
  int east;
  int west;
  int i, ii;
  int j = 1;

  answer = malloc(width * height * sizeof(int));
  for(i=0;i<height;i++)
    for(ii=0;ii<width;ii++)
	{
      answer[i*width+ii] = 0;
	  north = 0;
	  south = 0;
	  west = 0;
	  east = 0;
	  if(! isalpha( (unsigned char) puzzle[i*width+ii]))
	    continue;
	  if(i == 0 || ! isalpha( (unsigned char) puzzle[(i-1)*width+ii]) )
		north = 1;
	  if(i == height -1 || ! isalpha( (unsigned char) puzzle[(i+1)*width+ii]))
	    south = 1;
	  if(ii == 0 || !isalpha((unsigned char) puzzle[i*width+ii-1]))
        west = 1;
	  if(ii == width -1 || ! isalpha((unsigned char) puzzle[i*width+ii+1]))
	    east = 1;

	  if(north == 1 && south == 0)
	    answer[i*width+ii] = j++;
	  else if(west == 1 && east == 0)
	    answer[i*width+ii] = j++;
	}

  return answer;
}

unsigned long fletcher(char *data, int len )
{
  unsigned long sum1 = 0xffff, sum2 = 0xffff;
  int tlen;

  while (len) 
  {
    tlen = (len > 360) ? 360 : len;
    len -= tlen;
    do 
	{
      sum1 += *data++;
      sum2 += sum1;
     } while (--tlen);
     sum1 = (sum1 & 0xffff) + (sum1 >> 16);
     sum2 = (sum2 & 0xffff) + (sum2 >> 16);
  }
   /* Second reduction step to reduce sums to 16 bits */
   sum1 = (sum1 & 0xffff) + (sum1 >> 16);
   sum2 = (sum2 & 0xffff) + (sum2 >> 16);
   return sum2 << 16 | sum1;
}

/*
  callback to anneal
  clones a solution.
  Params; obj - the solution previously passed in to anneal
          ptr - the workspace
  Returns: a clone of obj
*/
static void *clone(void *obj, void *ptr)
{
  SOLUTION *sol = obj;
  SOLUTION *answer;

  answer = malloc(sizeof(SOLUTION));
  answer->N = sol->N;
  answer->words = malloc(sol->N * sizeof(char *));
  copy(answer, sol, ptr);
  return answer;
}

/*
  kill - callback to anneal, solution destructor
  Params: obj - object to destroy (previously returned from clone())
          ptr - workspace
*/
static void kill(void *obj, void *ptr)
{
  SOLUTION *sol = obj;

  free(sol->words);
  free(sol);
}

/*
  copy - callback to anneal
  copies one solution to another
  Params: dest - the destination solution
          src - the cource solution
		  ptr - workspace
  Returns: 0 for success, -1 for fail
*/
static int copy(void *dest, void *src, void *ptr)
{
  SOLUTION *copy = dest;
  SOLUTION *source = src;
  int i;

  for(i=0;i<source->N;i++)
    copy->words[i] = source->words[i];
  copy->pad = source->pad;
  return 0;
}

/*
  mutation function, callback to anneal
  Params: obj - the solution
          ptr - workspace
  Notes: mutation is to swap to words at random for greedy best match
*/
static void mutate(void *obj, void *ptr)
{
  SOLUTION *sol = obj;
  char *temp;
  int target;
 
  if( (rand() % 32) == 0)
	  sol->pad = rand() % 6;
  target = rand() % (sol->N - 1);
  temp = sol->words[target];
  sol->words[target] = sol->words[target+1];
  sol->words[target+1] = temp;
}

/*
  score fucntion, callback to anneal
  Params: obj - the solution
          ptr - workspace
  Returns: score - minimum is more optimal
  Notes: this fucntion does the bulk of the work ingenerating a solution
*/
static double score(void *obj, void *ptr)
{
  SOLUTION *sol = obj;
  WORKSPACE *wk = ptr;
  static int iter= 0;

  double answer = 0;
  int i;
  memset(wk->puzzle, 0, wk->width * wk->height);
  free(greedy(wk->puzzle, wk->width, wk->height, sol->words, sol->N, sol->pad, 0));
  for(i=0;i<wk->width*wk->height;i++)
    if(wk->puzzle[i] == 0)
	  answer += 1.0;
  if(wk->quiet == 0)
    printf("%d %f\n", iter++, answer);

  return answer;

}

/*
  perform greedy best fit of a list of words
  Params: puzzle - the puzzle reurned
          width - puzzle width
		  height - puzzle height
		  words - list of words
		  N - number of words
		  pad - spaces to skip for first word
		  Nused - return for number of words fitted
  Returns: 
*/
static ENTRY *greedy(char *puzzle, int width, int height, char **words, int N, int pad, int *Nused)
{
  int i, ii;
  int j = 1;
  ENTRY *answer;
  int nx;
  int x, y;
  int dir;

  answer = malloc(N * sizeof(ENTRY));
  for(i=0;i<N;i++)
    if(strlen(words[i]) + pad <= width)
	{
      strcpy(puzzle + pad, words[i]);
      break;
	}

  for(i=1;i<N;i++)
  {
    nx = placeword(puzzle, width, height, words[i], &x, &y, &dir);
    if(nx != 0)
	{
	  answer[j].word = i;
	  answer[j].x = x;
	  answer[j].y = y;
	  answer[j].dir = dir;
	
	  if(answer[j].dir == 0)
	  {
	    for(ii=0;words[i][ii];ii++)
	      puzzle[y * width + x + ii] = words[i][ii];
 	  }
	  else
	  {
	    for(ii=0;words[i][ii];ii++)
	      puzzle[ (y + ii) * width + x] = words[i][ii];
	  }
	  j++;
	}
  }

  if(Nused)
   *Nused = j;
  return 0;
}

/*
  place a word in optimal position for puzzle
  Params: puzzle - the puzzle
          width - puzzle width
		  height - puzzle height
		  word - word to place
		  x - return for x-coordinate
		  y - return for y co-ordiante
          dir - return for direction
  Returns: true if word placed, else false
  Notes: Rules are the word must fit in the grid without joining onto anohter
         word, and must share at least one letter with another word.
*/
static int placeword(char *puzzle, int width, int height, char *word, int *x, int *y, int *dir)
{
  int i, ii, iii;
  int ncrosses;
  int maxcrosses = 0;
  int bestx = 0;
  int besty = 0;
  int bestdir = 0;
  int wlen;

  wlen = strlen(word);
  for(i=0;i<height;i++)
    for(ii=0;ii<width-wlen+1;ii++)
	{
	   for(iii=0;iii<wlen;iii++)
	     if(puzzle[i*width+ii+iii] != 0 && puzzle[i*width+ii+iii] != word[iii])
		   break;
	   if(iii != wlen)
	     continue;

	   ncrosses = 0;
	  
       for(iii=0;iii<wlen;iii++)
		 if(puzzle[i*width+ii+iii] == word[iii])
		 {
		   if(ii + iii == width-1 || puzzle[i*width+ii+iii+1] == 0)
		     ncrosses++;
		 }
	   
	   if(i > 0)
	   {
	     for(iii=0;iii<wlen;iii++)
	       if(puzzle[(i-1)*width+ii+iii] != 0 && puzzle[i*width+ii+iii] == 0)
		     ncrosses = 0;
	   }
	   if(i < height-1)
	   {
	     for(iii=0;iii<wlen;iii++)
	       if(puzzle[(i+1)*width+ii+iii] != 0 && puzzle[i*width+ii+iii] == 0)
		     ncrosses = 0;
	   }
	   if(ii > 0 && puzzle[i*width+ii-1] != 0)
	     ncrosses = 0;
	   if(ii < width-wlen && puzzle[i*width+ii+wlen] != 0)
	     ncrosses = 0;
	   
	  
	   if(ncrosses > maxcrosses)
	   {
         maxcrosses = ncrosses;
		 bestx = ii;
         besty = i;
		 bestdir = 0;
	   }
	   
	    
	}
  
  for(i=0;i<height-wlen+1;i++)
    for(ii=0;ii<width;ii++)
	{
	   for(iii=0;iii<wlen;iii++)
	      if(puzzle[(i+iii)*width+ii] != 0 && puzzle[(i+iii)*width+ii] != word[iii])
		    break;
	   if(iii != wlen)
	     continue;

	   ncrosses = 0;
       for(iii=0;iii<wlen;iii++)
		 if(puzzle[(i+iii)*width+ii] == word[iii])
		 {
		   if(i+iii == height -1 || puzzle[(i+iii+1)*width+ii] == 0)
		     ncrosses++;
		 }
	   
	   if(ii > 0)
	   {
         for(iii=0;iii<wlen;iii++)
		   if(puzzle[(i+iii)*width+ii-1] != 0 && puzzle[(i+iii)*width+ii] == 0)
		     ncrosses = 0;
	   }
	   if(ii < width-1)
	   {
	     for(iii=0;iii<wlen;iii++)
		   if(puzzle[(i+iii)*width+ii+1] != 0 && puzzle[(i+iii)*width+ii] == 0)
		     ncrosses = 0;
	   }

	   if( i > 0 && puzzle[(i-1) * width + ii] != 0)
		   ncrosses = 0;
	   if(i < height - wlen && puzzle[(i+wlen) * width + ii] != 0)
	     ncrosses = 0;
	  
	   if(ncrosses > maxcrosses)
	   {
         maxcrosses = ncrosses;
		 bestx = ii;
         besty = i;
		 bestdir = 1;
	   }
	    
	}

	if(maxcrosses > 0)
	{
	  *x = bestx;
	  *y = besty;
	  *dir = bestdir;
	  return 1;
	}
	else
	{
	  *x = -1;
	  *y = -1;
	  *dir = -1;
	  return 0;
	}
}


