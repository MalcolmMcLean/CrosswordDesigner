/*  POTM ENTRY:	jigsaw						*/
/*  Your Name:	Franz Korntner					*/
/*  Your email:	fkorntne@hiscom.nl				*/
/*              korntner@xs4all.nl				*/
/*  To:		enter@potm.ffast.att.com			*/
/*  Subject:	C R O Z Z L E					*/

/*
**  Hello Fred. This is my entry with some additional comments and
**  some modifications to make it more portable. I haven't changed
**  the algoritm in any way whatsoever.
**  Because the source will be made available to the public, I've
**  included a copyright message to state that the source is free
**  and not public-domain. I'm sure thats alright with you.
*/
/* 
** Code modifed by Malcolm McLean for use as a component - took out
** non-ANSI time handling and made function accept wordlist as
** a parameter. Also changed code to cope with crosswords where width
** != height.
** There also seems to be a bug - for some reason the initial grid gets
** a bad level2xy value. Put it guard code to work around that. 
**
** LINKMAX was defined as the wrong value
 */
/*
   jigsaw, to create crossword puzzle grids
   Copyright 1996 Franz Korntner <fkorntne@hiscom.nl> <korntner@xs4all.nl>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
**  *** For your pleasure only ***
**
**  The code can produce symmetrical grids by setting the macro
**  SYMMETRICAL to 1. Sadly enough not as many words will fit
**  into a symmetrical grid than into a non-symmetrical grid,
**  so NODEMAX must be increased (say with 1000) to get better results.
*/
 
/* Configuring parameters */

#define TIMEMAX		(10*60-15)	/* 10 minute limit		*/
#define DEBUG		0		/* 0=Off 1=On 2=Verbose		*/
#define NODEMAX		1500		/* 500=Fast 1500=Normal		*/
#define SYMMETRICAL	0		/* 0=No 1=Yes			*/
#define MALLOC_BROKEN	0		/* 0=No 1=Yes (See below)	*/

/*
** It seems that malloc() under Linux is broken. I could only allocate
** a couple of thosand nodes before my program crashed. A solution was to
** allocate a BIG chunk of memory and split it up manually. This way about
** 10500 nodes could be allocated.
**
** I think that's all in the past. MALLOC_BROKEN is now itself broken.
** (Malcolm)
**
*/

/* Start of program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

static int GRIDWIDTH;      /* grid dimensions, including border */
static int GRIDHEIGHT;
#define WORDMAX 256			/* # words			*/
#define WORDLENMAX 32			/* # length of word		*/
#define LINKMAX (WORDMAX * WORDLENMAX * 3) /* links */
#define ADJMAX 128			/* # unaccounted adjecent chars	*/
#define SCOREMAX 1000			/* Spead for hashing		*/

#define BASE ('a'-1)			/* Word conversion		*/
#define STAR   (28)			/* Word delimiters		*/
#define FREE   (31)			/* Unoccupied grid cell		*/
#define TODOH 1				/* Hint: Hor. word can be here	*/
#define TODOV 2				/* Hint: Ver. word can be here	*/
#define BORDER 4			/* Cell is grid border		*/

#define ISSTAR(C)   ((C)==STAR)		/* Test the above		*/
#define ISFREE(C)   ((C)==FREE)
#define ISCHAR(C)   ((C)< STAR)
#define ISBORDER(C) ((C)&BORDER)

#define BITSET(M,B) ((M).s[(B)>>3] |=  (1<<((B)&7))) /* BitSet		*/
#define BITCLR(M,B) ((M).s[(B)>>3] &= ~(1<<((B)&7))) /*   manipulation	*/
#define INSET(M,B)  ((M).s[(B)>>3] &   (1<<((B)&7)))

typedef struct {			/* Bitset containing 256 bits	*/
  unsigned char s[32];
} SET;

typedef struct node {
  struct node	*next;			/*				*/
  int		seqnr;			/* For diagnostics		*/
  SET		words;			/* Summary of placed words	*/
  int		numword,numchar,numconn;/* Statistics			*/
  unsigned int	hash;			/* Duplicate detection		*/
  int		firstlevel, lastlevel;	/* Grids hotspot		*/
  float		score;			/* Will it survive?		*/
  signed char	symdir;			/* Force symmetry		*/
  short		symxy;			/* 				*/
  short		symlen;			/* 				*/
  int		numadj;			/* # unprocessed char pairs	*/
  signed char	adjdir[ADJMAX];		/* Pair's direction		*/
  short		adjxy[ADJMAX];		/* Pair's location		*/
  short		adjl[ADJMAX];		/* Pair's wordlist		*/
  unsigned char	*grid; //[GRIDMAXSIZE*GRIDMAXSIZE];	/* *THE* grid			*/
  unsigned char	*attr; //[GRIDMAXSIZE*GRIDMAXSIZE];	/* Grid hints			*/
} node;

struct link {
  short		next;
  short		w;				/* What's the word	*/
  short		ofs;				/* Were's in the word	*/
};


/* The external word list */
static unsigned char	wordbase[WORDMAX][WORDLENMAX];	/* Converted wordlist	*/
static int		wlen[WORDMAX];			/* Length of words	*/
static int		numword;			/* How many		*/

/* Where are 1,2,3 long character combinations */
static short		links1[32];			/* 1-char wordlist	*/
static short		links2[32][32];			/* 2-char wordlist	*/
static short		links3[32][32][32];		/* 3-char wordlist	*/
static struct link	linkdat[LINKMAX];		/* Body above wordlist	*/
static int		numlinkdat;			/* How many		*/

/* Hotspot pre-calculations */
static short		*xy2level; //[GRIDMAXSIZE*GRIDMAXSIZE];	/* distance 0,0 to x,y	*/
static short		*level2xy; //[GRIDMAXSIZE*2];		/* inverse		*/

/* Node administration */
static node		*freenode;			/* Don't malloc() too much */
static int		numnode, realnumnode;		/* Statistics		*/
static node		solution;			/* What are we doing?	*/
static node		*scores[SCOREMAX];		/* Speed up hashing	*/

/* Diagnostics */
static int		seqnr;
static int		hashtst, hashhit;
static int		nummalloc;
static int		numscan;
static int		flg_dump;

/* What's left */
static time_t    tick;   /* time we started the function */


/*
** Display the grid. Show disgnostics info in verbose mode
*/

static void dump_grid (node *d)
{
  int x, y;
         
  /* Show grid as Fred would like to see it */
  for (y=1; y<GRIDHEIGHT-1; y++) {
    for (x=1; x<GRIDWIDTH-1; x++)
      if (ISCHAR(d->grid[x+y*GRIDWIDTH]))
        printf ("%c", d->grid[x+y*GRIDWIDTH]+BASE);
      else
        printf("-");
    printf("\n");
  }

}
  
static int timeout(void)
{
 time_t tock;

 tock = time(0);
 if(difftime(tock, tick) > TIMEMAX)
   return 1;
 return 0;
}


/*
** Get a node, reusing free'ed nodes.
*/

static node *mallocnode (void)
{
  node *d;

  d = freenode;
  if (d == NULL) {
#if MALLOC_BROKEN == 0
    d = (node*) malloc (sizeof(node));
    memset(d, 0, sizeof(node));
    d->grid = malloc(GRIDWIDTH * GRIDHEIGHT);
    memset(d->grid, 0, GRIDWIDTH * GRIDHEIGHT);
    d->attr = malloc(GRIDWIDTH * GRIDHEIGHT);
    memset(d->attr, 0, GRIDWIDTH * GRIDHEIGHT);
    d->next = 0;
    nummalloc++;
#endif
  } else
    freenode = d->next;

  return d;
}

static node *copynode(node *d)
{
  node *answer;
  unsigned char *grid;
  unsigned char *attr;

  answer = mallocnode();
  if(!answer)
    return 0;
  grid = answer->grid;
  attr = answer->attr;
  memcpy(answer, d, sizeof(node));
  answer->grid = grid;
  answer->attr = attr;
  memcpy(answer->grid, d->grid, GRIDWIDTH*GRIDHEIGHT);
  memcpy(answer->attr, d->attr, GRIDWIDTH*GRIDHEIGHT);
  
  return answer;
}

static void freeanode(node *d)
{
  if(d)
  {
    free(d->grid);
    free(d->attr);
    free(d);
  }
}

static void freenodelist(node  *d)
{
  node *next;

  while(d)
  {
    next = d->next;
  
    freeanode(d);
    d = next;
    nummalloc--;
  }
 
}

static void add_node (node *d)
{
int i;
node **prev, *next;

  /* Evaluate grids score */
  d->score = ((float)d->numconn)/d->numchar;

  /* Insert grid into sorted list, eliminating duplicates */
  i = (int) (d->score*(SCOREMAX-1));
  if (i<0) i=0;
  if (i>=SCOREMAX) i=SCOREMAX-1;
  prev = &scores[i];
  next = scores[i];
  for (;;) {
    if (next == NULL) break;
    if (d->score > next->score) break;
    if (d->score == next->score && d->hash >= next->hash) break;
    prev = &next->next;
    next = next->next;
  }

  /* Test if entry is duplicate */
  while (next && d->score == next->score && d->hash == next->hash) {
    hashtst++;
    if (memcmp(d->grid, next->grid, GRIDWIDTH * GRIDHEIGHT) == 0) {
      d->next = freenode;
      freenode = d;
      return;
    }
    hashhit++;
    prev = &next->next;
    next = next->next;
  }

  /* Diagnostics */
  if (flg_dump > 1)
    dump_grid (d);

  /* Enter grid into list */
  d->seqnr = seqnr++;
  d->next = next;
  (*prev) = d;
  if (d->numadj == 0)
    numnode++;
  realnumnode++;
}


/*
** The next two routines test if a given word can be placed in the grid.
** If a new character will be adjecent to an existing character, check
** if the newly formed character pair exist in the wordlist (it doesn't
** matter where).
*/

static int test_hword (node *d, int xybase, int word)
{
unsigned char *p, *grid;
int l;

  /* Some basic tests */
  if (xybase < 0 || xybase+wlen[word] >= GRIDWIDTH*GRIDHEIGHT+1) return 0;

#if SYMMETRICAL
  /* How about star's */
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-xybase]))
    return 0;
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]-1)]))
    return 0;
#endif

  /* Will new characters create conflicts */
  for (grid=d->grid+xybase,p=wordbase[word]; *p; grid++,p++) {
    if (*grid == *p)
      continue; /* Char already there */
    if (!ISFREE(*grid))
      return 0; /* Char placement conflict */
    if (ISSTAR(*p))
      continue; /* Skip stars */
    if (!ISCHAR(grid[-GRIDWIDTH]) && !ISCHAR(grid[+GRIDWIDTH]))
      continue; /* No adjecent chars */

    if (ISFREE(grid[-GRIDWIDTH]))
      l = links2[*p][grid[+GRIDWIDTH]];
    else if (ISFREE(grid[+GRIDWIDTH]))
      l = links2[grid[-GRIDWIDTH]][*p];
    else
      l = links3[grid[-GRIDWIDTH]][*p][grid[+GRIDWIDTH]];
    if (l == 0)
      return 0;
  }

  /* Word can be placed */
  return 1;
}

static int test_vword (node *d, int xybase, int word)
{
unsigned char *p, *grid;
int l;

  /* Some basic tests */
  if (xybase < 0 || xybase+wlen[word]*GRIDWIDTH >= GRIDWIDTH*GRIDHEIGHT+GRIDWIDTH) return 0;

#if SYMMETRICAL
  /* How about star's */
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-xybase]))
    return 0;
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]*GRIDWIDTH-GRIDWIDTH)]))
    return 0;
#endif

  /* Will new characters create conflicts */
  for (grid=d->grid+xybase,p=wordbase[word]; *p; grid+=GRIDWIDTH,p++) {
    if (*grid == *p)
      continue; /* Char already there */
    if (!ISFREE(*grid))
      return 0; /* Char placement conflict */
    if (ISSTAR(*p))
      continue; /* Skip stars */
    if (!ISCHAR(grid[-1]) && !ISCHAR(grid[+1]))
      continue; /* No adjecent characters */

    if (ISFREE(grid[-1]))
      l = links2[*p][grid[+1]];
    else if (ISFREE(grid[+1]))
      l = links2[grid[-1]][*p];
    else
      l = links3[grid[-1]][*p][grid[+1]];
    if (l == 0)
      return 0;
  }

  /* Word can be placed */
  return 1;
}


/*
** The next two routines will place a given word in the grid. These routines
** also performs several sanity checks to make sure the new grid is worth
** it to continue with. If a newly placed character is adjecent to an 
** existing character, then that pair must be part of a word that can
** be physically placed. If multiple character pairs exist, then no check
** is done to determine if those words (of which the pairs are part) can
** be adjecent. That is done later as these grids are not counted against
** NODEMAX.
*/

static int place_hword (node *data, int xybase, int word)
{
node *d=data;
unsigned char *p, *grid, *attr;
int i, l, xy;
int newnumadj;
struct link *ld;

  /* Can word be placed */
  if (INSET(d->words, word)) return 0;
  if (xybase < 0 || xybase+wlen[word] >= GRIDWIDTH*GRIDHEIGHT+1) return 0;

#if SYMMETRICAL
  /* How about star's */
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-xybase]))
    return 0;
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]-1)]))
    return 0;
#endif

  /* check character environment */
  newnumadj = d->numadj;
  for (xy=xybase,grid=d->grid+xy,p=wordbase[word]; *p; grid++,xy++,p++) {
    if (*grid == *p)
      continue; /* Char already there */
    if (!ISFREE(*grid))
      return 0; /* Char placement conflict */
    if (ISSTAR(*p))
      continue; /* Skip stars */
    if (!ISCHAR(grid[-GRIDWIDTH]) && !ISCHAR(grid[+GRIDWIDTH]))
      continue; /* No adjecent chars */

    if (ISFREE(grid[-GRIDWIDTH])) {
      d->adjxy [newnumadj] = xy;
      d->adjl  [newnumadj] = links2[*p][grid[+GRIDWIDTH]];
    } else if (ISFREE(grid[+GRIDWIDTH])) {
      d->adjxy [newnumadj] = xy-GRIDWIDTH;
      d->adjl  [newnumadj] = links2[grid[-GRIDWIDTH]][*p];
    } else {
      d->adjxy [newnumadj] = xy-GRIDWIDTH;
      d->adjl  [newnumadj] = links3[grid[-GRIDWIDTH]][*p][grid[+GRIDWIDTH]];
    }
    if (d->adjl[newnumadj] == 0 || newnumadj == ADJMAX-1)
      return 0;
    d->adjdir[newnumadj++] = 'V';
  }

  /* Test if new adj's really exist */
  for (i=d->numadj; i<newnumadj; i++) {
    for (l=d->adjl[i]; l; l=ld->next) {
      ld = &linkdat[l];
      if (test_vword (d, d->adjxy[i]+ld->ofs*GRIDWIDTH, ld->w))
        break;
    }
    if (l == 0) return 0;
    d->adjl[i] = l;
  }

  /* Get a new grid */
  d = copynode(data);
  if(!d)
    return -1;

  /* Place word */
  BITSET(d->words, word);
  d->numword++;
  d->numadj = newnumadj;
  for (xy=xybase,grid=d->grid+xy,attr=d->attr+xy,p=wordbase[word];
       *p;
       grid++,attr++,xy++,p++) {
    if (!ISSTAR(*p)) {
      *attr &= ~TODOH;

      /* Remove character pair hints that are part of the new word */
      for (i=0; i<d->numadj; i++)
        if (d->adjdir[i] == 'H' && d->adjxy[i] == xy) {
          d->adjdir[i] = d->adjdir[--d->numadj];
          d->adjxy [i] = d->adjxy [  d->numadj];
          d->adjl  [i] = d->adjl  [  d->numadj];
          break; /* There can be only one */
        }

      /* Place character */
      if (ISFREE(*grid)) {
        d->hash += (123456+xy)*(123456-*p); /* Neat, isn't it? */
        *attr |= TODOV;
        d->numchar++;
      } else {
        d->numconn++;
     }
    }
    *grid = *p;
  }

  /* Update hotspot */
  if (xy2level[xy-1] > d->lastlevel)
    d->lastlevel = xy2level[xy-1];

#if SYMMETRICAL
  /* Don't forget the symmetry */
  if (d->symdir == 0) {
    d->symdir = 'H';
    d->symxy  = GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]-1);
    d->symlen = wlen[word];
  } else
    d->symdir = 0;
#endif

  /* Save grid */
  add_node (d);
  return 1;
}

static int place_vword (node *data, int xybase, int word)
{
node *d=data;
unsigned char *p, *grid, *attr;
int i, xy, l;
int newnumadj;
struct link *ld;

  /* Some basic tests */
  if (INSET(d->words, word)) return 0;
  if (xybase < 0 || xybase+wlen[word]*GRIDWIDTH >= GRIDWIDTH*GRIDHEIGHT+GRIDWIDTH) return 0;

#if SYMMETRICAL
  /* How about star's */
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-xybase]))
    return 0;
  if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]*GRIDWIDTH-GRIDWIDTH)]))
    return 0;
#endif

  /* Check character environment */
  newnumadj = d->numadj;
  for (xy=xybase,grid=d->grid+xy,p=wordbase[word]; *p; grid+=GRIDWIDTH,xy+=GRIDWIDTH,p++) {
    if (*grid == *p)
      continue; /* Char already there */
    if (!ISFREE(*grid))
      return 0; /* Char placement conflict */
    if (ISSTAR(*p))
      continue; /* Skip stars */
    if (!ISCHAR(grid[-1]) && !ISCHAR(grid[+1]))
      continue; /* No adjecent chars */

    if (ISFREE(grid[-1])) {
      d->adjxy [newnumadj] = xy;
      d->adjl  [newnumadj] = links2[*p][grid[+1]];
    } else if (ISFREE(grid[+1])) {
      d->adjxy [newnumadj] = xy-1;
      d->adjl  [newnumadj] = links2[grid[-1]][*p];
    } else {
      d->adjxy [newnumadj] = xy-1;
      d->adjl  [newnumadj] = links3[grid[-1]][*p][grid[+1]];
    }
    if (d->adjl[newnumadj] == 0 || newnumadj == ADJMAX-1)
      return 0;
    d->adjdir[newnumadj++] = 'H';
  }

  /* Test if new adj's really exist */
  for (i=d->numadj; i<newnumadj; i++) {
    for (l=d->adjl[i]; l; l=ld->next) {
      ld = &linkdat[l];
      if (test_hword (d, d->adjxy[i]+ld->ofs, ld->w))
        break;
    }
    if (l == 0) return 0;
    d->adjl[i] = l;
  }

  /* Get a new grid */
  d = copynode(data);
  if(!d)
    return -1;

  /* Place word */
  BITSET(d->words, word);
  d->numword++;
  d->numadj = newnumadj;
  for (xy=xybase,grid=d->grid+xy,attr=d->attr+xy,p=wordbase[word];
       *p;
       grid+=GRIDWIDTH,attr+=GRIDWIDTH,xy+=GRIDWIDTH,p++) {
    if (!ISSTAR(*p)) {
      *attr &= ~TODOV;

      /* Remove character pair hints that are part of the new word */
      for (i=0; i<d->numadj; i++)
        if (d->adjdir[i] == 'V' && d->adjxy[i] == xy) {
          d->adjdir[i] = d->adjdir[--d->numadj];
          d->adjxy [i] = d->adjxy [  d->numadj];
          d->adjl  [i] = d->adjl  [  d->numadj];
          break; /* There can be only one */
        }

      /* Place character */
      if (ISFREE(*grid)) {
        *attr |= TODOH;
        d->hash += (123456+xy)*(123456-*p); /* Neat, isn't it? */
        d->numchar++;
      } else {
        d->numconn++;
      }
    }
    *grid = *p;
  }

  /* Update hotspot */
  if (xy2level[xy-GRIDWIDTH] > d->lastlevel)
    d->lastlevel = xy2level[xy-GRIDWIDTH];

#if SYMMETRICAL
  /* Don't forget the symmetry */
  if (d->symdir == 0) {
    d->symdir = 'V';
    d->symxy  = GRIDWIDTH*GRIDHEIGHT-1-(xybase+wlen[word]*GRIDWIDTH-GRIDWIDTH);
    d->symlen = wlen[word];
  } else
    d->symdir = 0;
#endif

  /* Save grid */
  add_node (d);
  return 1;
}


/*
** Scan a grid and place a word. To supress an exponential growth of
** generated grids, we can be very fussy when chosing which word to
** place. I have chosen to fill the grid from top-left to bottom-right
** making sure the newly placed words fit tightly to the already placed
** words. If this is not possible, then I choose just one word such that
** the first letter is nearest to the start of the hotspot regeon.
** This sounds easy but it took me quite some time to figure it out.
*/

static int scan_grid (node *d)
{
unsigned char *grid, *attr;
int xy, l, cnt, tstxy, level;
struct link *ld;
int hasplace, hasfree;
int answer = 0;
int placed;

#if SYMMETRICAL
  /* Don't forget the symmetry */
  if (d->symdir == 'H') {
    for (w=numword-1; w>=0; w--)
      if (!INSET(d->words, w) && wlen[w] == d->symlen)
        answer = place_hword (d, d->symxy, w);
    return answer;
  }
  if (d->symdir == 'V') {
    for (w=numword-1; w>=0; w--)
      if (!INSET(d->words, w) && wlen[w] == d->symlen)
        answer = place_vword (d, d->symxy, w);
    return answer;
  }
#endif

  /* locate unaccounted adjecent cells */
  if (d->numadj > 0) {
    xy  = d->adjxy[--d->numadj];
    if (d->adjdir[d->numadj] == 'H') {
      for (l=d->adjl[d->numadj]; l; l=ld->next) {
        ld = &linkdat[l];
        answer = place_hword (d, xy+ld->ofs, ld->w);
        if(answer == -1)
          return answer;
      }
    } else {
      for (l=d->adjl[d->numadj]; l; l=ld->next) {
        ld = &linkdat[l];
        answer = place_vword (d, xy+ld->ofs*GRIDWIDTH, ld->w);
        if(answer == -1)
          return answer;
      }
    }
    return answer;
  }


  /* Nominate grid for final result */
  if (d->numword > solution.numword)
    memcpy (&solution, d, sizeof(node));

  /* Sweep grid from top-left to bottom-right corner */
  for (level=d->firstlevel; level<=d->lastlevel&&level<=GRIDWIDTH+GRIDHEIGHT-4; level++) {

#if !SYMMETRICAL
    /* Locate 'tight' words */
    hasplace = 0;
    for (xy=level2xy[level],grid=d->grid+xy,attr=d->attr+xy;
         !ISBORDER(*attr);
         xy+=GRIDWIDTH-1,grid+=GRIDWIDTH-1,attr+=GRIDWIDTH-1) {
      if (*attr&TODOH) {
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs; 
	  if(tstxy < 0 || tstxy >= GRIDWIDTH * GRIDHEIGHT)
            continue;
      
          if (xy2level[tstxy] == d->firstlevel)
	  {
            placed = place_hword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
	    hasplace += placed;
          }
        }
      }
      if (*attr&TODOV) {
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs*GRIDWIDTH;
          if(tstxy < 0 || tstxy >= GRIDWIDTH * GRIDHEIGHT)
            continue;
          if (xy2level[tstxy] == d->firstlevel)
	  {
            placed =  place_vword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            hasplace += placed;
          }
        }
      }
      if (hasplace)
        return answer;
    }
#endif


    /* Locate 'adjecent' word */
    hasplace = 0;
    for (xy=level2xy[level],grid=d->grid+xy,attr=d->attr+xy;
         !ISBORDER(*attr);
         xy+=GRIDWIDTH-1,grid+=GRIDWIDTH-1,attr+=GRIDWIDTH-1) {
      if (*attr&TODOH) {
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs;
          if (tstxy >= 0 && tstxy < GRIDWIDTH*GRIDHEIGHT && ISSTAR(d->grid[tstxy]))
	  {
            placed =  place_hword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            hasplace += placed;
          }
#if SYMMETRICAL
          if (ISSTAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(tstxy+wlen[ld->w]-1)]))
	  {
            placed =  place_hword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            hasplace += placed;
          }
#endif
        }
      }
     
      if (*attr&TODOV) {
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs*GRIDWIDTH;
          if(tstxy >= 0 && tstxy < GRIDWIDTH * GRIDHEIGHT && ISSTAR(d->grid[tstxy]))
	  {
            placed =  place_vword (d, tstxy, ld->w);
            if(placed == -1)
	      return -1;
            hasplace += placed;
          }
#if SYMMETRICAL
          if (ISSTAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(tstxy+wlen[ld->w]*GRIDWIDTH-GRIDWIDTH)]))
	  {
            placed =  place_vword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            hasplace += placed;
          }
#endif
        
        }
      }
      if (hasplace)
        return answer;
    }

    /* Locate word fragments (just one word please) */
    hasplace = 0; 
    hasfree = 0;
    for (xy=level2xy[level],grid=d->grid+xy,attr=d->attr+xy;
         !ISBORDER(*attr);
         xy+=GRIDWIDTH-1,grid+=GRIDWIDTH-1,attr+=GRIDWIDTH-1) {
      if (ISFREE(*grid))
        hasfree = 1;
      if (*attr&TODOH) {
        cnt = 0;
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs;
          if (tstxy >= 0 && tstxy < GRIDWIDTH * GRIDHEIGHT &&!ISSTAR(d->grid[tstxy]))
	  {
            placed =  place_hword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            cnt += placed;
          }
#if !SYMMETRICAL
          if (cnt != 0)
            break;
#endif
        }
        if (cnt == 0) {
          /* Speed things up (Not 100% correct, but it's fast) */
          *attr &= ~TODOH;
          grid[-1] = STAR;
          grid[+1] = STAR;
#if SYMMETRICAL
          if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy-1)])) return; /* Arghh */
          d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy-1)] = STAR;
          if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy+1)])) return; /* Arghh */
          d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy+1)] = STAR;
#endif
        }
        hasplace += cnt;
      }
      if (*attr&TODOV) {
        cnt = 0;
        for (l=links1[*grid]; l; l=ld->next) {
          ld = &linkdat[l];
          tstxy = xy+ld->ofs*GRIDWIDTH;
          if (tstxy >= 0 && tstxy < GRIDWIDTH * GRIDHEIGHT && !ISSTAR(d->grid[tstxy]))
	  {
            placed =  place_vword (d, tstxy, ld->w);
            if(placed == -1)
              return -1;
            cnt += placed;
          }
#if !SYMMETRICAL
          if (cnt != 0)
            break;
#endif
        }
        if (cnt == 0) {
          /* Speed things up (Not 100% correct, but it's fast) */
          *attr &= ~TODOV;
          grid[-GRIDWIDTH] = STAR;
          grid[+GRIDWIDTH] = STAR;
#if SYMMETRICAL
          if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy-GRIDWIDTH)])) return; /* Arghh */
          d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy-GRIDWIDTH)] = STAR;
          if (ISCHAR(d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy+GRIDWIDTH)])) return; /* Arghh */
          d->grid[GRIDWIDTH*GRIDHEIGHT-1-(xy+GRIDWIDTH)] = STAR;
#endif
        }
        hasplace += cnt;
      }
      if (hasplace)
        return answer;
    }

    /* Update hotspot */
    if (!hasfree) 
      d->firstlevel = level+1;
  }

  return answer;
}

static void kick_donkey (void)
{
  node *d, *todonode;
  int i;
  int err;

  for (;;) {
    /* setup up some debugging statistics */
    realnumnode = numnode = numscan = 0;
    hashtst = hashhit = 0;

    /* gather all nodes into a single list with highest score first */
    d=todonode = NULL;
    for (i=SCOREMAX-1; i>=0; i--)
      if (scores[i]) {
        if (todonode == NULL) 
          d=todonode=scores[i];
        else 
          d->next = scores[i];
        while (d->next) d=d->next;
        scores[i] = NULL;
      }

    /* Ok babe, lets go!!! */
    while (todonode) {
      d = todonode;
      todonode = d->next;
      if (d->numadj > 0 || numnode<NODEMAX) {
        if (d->numadj == 0) numscan++;
        err = scan_grid (d);
        if(err == -1)
          return;
      }
      d->next = freenode;
      freenode = d;
    }

    /* Test for timeouts */
    if( timeout() )
      return;

#if DEBUG
    fprintf(stderr, "%s word:%2d score:%f level:%2d/%2d node:%4d/%4d/%4d hash:%3d/%3d\n", 
            elapsedstr(), solution.numword, solution.score, 
            solution.firstlevel, solution.lastlevel, numscan, numnode, 
            realnumnode, hashtst, hashhit);
    if (flg_dump) dump_grid (&solution);
#endif

    /* Ass kicked? */
    if (realnumnode == 0)
      break;
  }
}

static void setwordlist(char **list, int N)
{
  int i, ii;
  int j = 0;

  for(i=0;i<N;i++)
  {
    if(strlen(list[i]) >= WORDLENMAX-4)
      continue;
    for(ii=0;list[i][ii];ii++)
      if(!isalpha((unsigned char) list[i][ii]))
	 break;
    if(list[i][ii])
      continue;

    wordbase[j][0] = STAR;
    for(ii=0;list[i][ii];ii++)
      wordbase[j][ii+1] = tolower((unsigned char) list[i][ii]) - BASE;
    wordbase[j][ii+1] = STAR;
    wordbase[j][ii+2] = 0;
    wlen[j] = ii+2;
    j++;
    if(j >= WORDMAX)
      break;
  }
  numword = j;
}

static void calculatelinks(void)
{
  int i, w, done;
  unsigned char *p;

  /*
  ** Calculate links. These are indexes on the wordlist which are used
  ** to quickly locate 1,2,3 long letter sequences within the words.
  */
  numlinkdat=1;
  memset (links1, 0, sizeof(links1));
  memset (links2, 0, sizeof(links2));
  memset (links3, 0, sizeof(links3));
  for (i=0,done=0; !done && i<WORDLENMAX-2; i++) {
    done = 1;
    for (w=numword-1; w>=0; w--) {
      p = wordbase[w];
      if (i>=0 && i<=wlen[w]-3) {
        /* With delimiters */
        linkdat[numlinkdat].w = w;
        linkdat[numlinkdat].ofs = -i;
        linkdat[numlinkdat].next = links3[p[i+0]][p[i+1]][p[i+2]];
        links3[p[i+0]][p[i+1]][p[i+2]] = numlinkdat++;
        done = 0;
      }
      if (i>=0 && i<=wlen[w]-2) {
        /* With delimiters */
        linkdat[numlinkdat].w = w;
        linkdat[numlinkdat].ofs = -i;
        linkdat[numlinkdat].next = links2[p[i+0]][p[i+1]];
        links2[p[i+0]][p[i+1]] = numlinkdat++;
        done = 0;
      }
      if (i>0 && i<=wlen[w]-2) {
        /* Without delimiters */
        linkdat[numlinkdat].w = w;
        linkdat[numlinkdat].ofs = -i;
        linkdat[numlinkdat].next = links1[p[i]];
        links1[p[i]] = numlinkdat++;
        done = 0;
      }
    }
  }
}

static void nodetooutput(char *grid, node *d)
{
  int x, y;
  int j = 0;
 
  /* Show grid as Fred would like to see it */
  for (y=1; y<GRIDHEIGHT-1; y++) {
    for (x=1; x<GRIDWIDTH-1; x++)
      if (ISCHAR(d->grid[x+y*GRIDWIDTH]))
        grid[j++] = toupper((unsigned char) d->grid[x+y*GRIDWIDTH]+BASE);
      else
        grid[j++] = 0;
  }
}

int jigsawcrossword(char *grid, int width, int height, char **list, int N)
{
  int i, w;
  int x, y;
  node *d;
  node *base;

  if(width < 4 || width > 100 - 2 || height < 4 || height > 100-2)
    return -1;
  GRIDWIDTH = width + 2;
  GRIDHEIGHT = height + 2;
  tick = time(0);

  for(i=0;i<SCOREMAX;i++)
	  scores[i] = NULL;

  solution.numword = 0;

  setwordlist(list, N);
  calculatelinks();
  xy2level = malloc(GRIDWIDTH *GRIDHEIGHT * sizeof(short));
  level2xy = malloc( (GRIDWIDTH+GRIDHEIGHT) * sizeof(short));

   
  /* init grid administration */
  freenode = NULL;
#if MALLOC_BROKEN
  nummalloc = 6000;
  d= malloc (nummalloc*sizeof(node));
  base = d;
  if (d == NULL)
    nummalloc = 0;
  else
    for(i=0; i<nummalloc; i++) {
      d[i].next = freenode;
      freenode = d+i;
    }
#else
 
#endif

   
  /* Do some hotspot pre-calculations */
  for (i=GRIDWIDTH*GRIDHEIGHT-1; i>=0; i--)
    xy2level[i] = (i%GRIDWIDTH)+(i/GRIDWIDTH);
  for (i=0; i<GRIDWIDTH+GRIDHEIGHT; i++) {
    if (i<GRIDWIDTH) {
      level2xy[i] = i+GRIDWIDTH-1;
    } else {
      level2xy[i] = GRIDWIDTH*GRIDHEIGHT-(GRIDHEIGHT-3-i+GRIDWIDTH)*GRIDWIDTH-2;
    }
  }

  /* create an initial grid */
  d = mallocnode();
  for (y=GRIDHEIGHT-1; y>=0; y--)
    for (x=GRIDWIDTH-1; x>=0; x--) {
      d->grid[x+y*GRIDWIDTH] = STAR;
      d->attr[x+y*GRIDWIDTH] = BORDER;
    }
  for (y=GRIDHEIGHT-2; y>0; y--)
    for (x=GRIDWIDTH-2; x>0; x--) {
      d->grid[x+y*GRIDWIDTH] = FREE;
      d->attr[x+y*GRIDWIDTH] = 0;
    }

  /* Place all the words for starters */
#if SYMMETRICAL
  /* work from the middle out */
  for (w=numword-1; w>=0; w--)
    if (wlen[w]>=5) {
      d->hash = 0;
      place_hword (d, (GRIDWIDTH/2+2-wlen[w])+(GRIDHEIGHT/2)*GRIDWIDTH, w);
    }
#else
  /* work from top-left to bottom-right */
  for (w=numword-1; w>=0; w--) {
    d->firstlevel = 2;
    place_hword (d, 0+1*GRIDWIDTH, w);
  }
#endif
  d->next = freenode;
  freenode = d;

  /* Here we go */
  kick_donkey();

  nodetooutput(grid, &solution);

  free(level2xy);
  free(xy2level);
#if MALLOC_BROKEN
  free(d);
  free(base);
#else
  freenodelist(freenode);  
#endif

  return 0;
}
  
static char *wordlist[51] =
  {
    "CAMBRIDGE",
    "SACREDDRAUGHTS",
    "CHRISTS",
    "CHURCHILL",
    "CLARECOLLEGE",
    "CLAREHALL",
    "CORPUS",
    "DARWIN",
    "DOWNING",
    "EMMANUEL",
    "FITZWILLIAM",
    "GIRTON",
    "CAIUS",
    "HOMERTON",
    "HUGHESHALL",
    "JESUS",
    "KINGS",
    "CAVENDISH",
    "MAGDALENE",
    "NEWHALL",
    "NEWNHAM",
    "PEMBROKE",
    "PETERHOUSE",
    "QUEENS",
    "ROBINSON",
    "CATS",
    "EDMUNDS",
    "JOHNS",
    "SELWYN",
    "SIDNEYSUSSEX",
    "TRINITY",
    "TRINITYHALL",
    "WOLFSON",
    "SEMPEREADEM",
    "PICKEREL",
    "GRAPES",
    "COUNTY",
    "CASTLE",
    "CARLTON",
    "FREEPRESS",
    "MITRE",
    "BARONOFBEEF",
    "BLUE",
    "TERIAKI",
    "AKITERI",
    "MIDSUMMERHOUSE",
    "GREATSTMARYS",
    "ADC",
    "ICE",
    "NEWTON",
    "LADYMARGARET",
  };

 int genjigsawmain(void)
 {
   char grid[20*20];
   int i, ii;
   int width = 10;
   int height = 20;


   jigsawcrossword(grid, width, height, wordlist, 51); 
   for(i=0;i<height;i++)
   {
     for(ii=0;ii<width;ii++)
       printf("%c", grid[i*width+ii] ? grid[i*width+ii] : ' ');
     printf("\n");
   }  
   return 0;
 }
