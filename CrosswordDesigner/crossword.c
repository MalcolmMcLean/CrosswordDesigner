#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "crossword.h"

static int renumber(CROSSWORD *cw);
static char **getwordsacross(char *solution, unsigned char *grid, int width, int height, int *N);
static char **getwordsdown(char *solution, unsigned char *grid, int width, int height, int *N);
static int downwordstart(unsigned char *grid, int width, int height, int x, int y);
static int acrosswordstart(unsigned char *grid, int width, int height, int x, int y);
static void freelist(char **str, int N);
static char *mystrdup(const char *str);

CROSSWORD *createcrossword(int width, int height)
{
  CROSSWORD *answer;
  int i;


  answer = malloc(sizeof(CROSSWORD));
  if(!answer)
    goto error_exit;
  answer->width = width;
  answer->height = height;
  answer->solution  = 0;
  answer->grid = 0;
  answer->numbers = 0;
  answer->wordsacross = 0;
  answer->numbersacross = 0;
  answer->wordsdown = 0;
  answer->numbersdown = 0;
  answer->cluesacross = 0;
  answer->cluesdown = 0;
  answer->acrossclues_grid = 0;
  answer->downclues_grid = 0;
  answer->Nacross = 0;
  answer->Ndown = 0;

  answer->title = 0;
  answer->author = 0;
  answer->editor = 0;
  answer->publisher = 0;
  answer->date = 0;
  answer->copyright = 0;

   
  answer->solution = malloc(width * height);
  if(!answer->solution)
	  goto error_exit;
  answer->grid = malloc(width * height);
  if(!answer->grid)
	  goto error_exit;
  answer->numbers = malloc(width * height * sizeof(int));
  if(!answer->numbers)
    goto error_exit;
  memset(answer->grid, 0, width * height);
  memset(answer->solution, ' ', width * height);
  for(i=0;i<width*height;i++)
	  answer->numbers[i] = 0;
  answer->acrossclues_grid = malloc(width * height * sizeof(char *));
  if(!answer->acrossclues_grid)
	  goto error_exit;
  for(i=0;i<width*height;i++)
	  answer->acrossclues_grid[i] = 0;
  answer->downclues_grid = malloc(width * height * sizeof(char *));
  if(!answer->downclues_grid)
	  goto error_exit;
  for(i=0;i<width *height;i++)
	  answer->downclues_grid[i] = 0;
  return answer;
error_exit:
   killcrossword(answer);
   return 0;
}

CROSSWORD *crossword_clone(CROSSWORD *cw)
{
	CROSSWORD *answer;
	int i;

	answer = createcrossword(cw->width, cw->height);
	for (i = 0; i < cw->height*cw->width; i++)
		crossword_setcell(answer, i % cw->width, i / cw->width, cw->grid[i] ? cw->solution[i] : 0);
	for (i = 0; i < cw->Nacross;i++)
		if (cw->cluesacross[i])
			answer->cluesacross[i] = mystrdup(cw->cluesacross[i]);
	for (i=0; i < cw->Ndown;i++)
		if (cw->cluesdown[i])
			answer->cluesdown[i] = mystrdup(cw->cluesdown[i]);

	if (cw->title)
		answer->title = mystrdup(cw->title);
	if (cw->author)
		answer->author = mystrdup(cw->author);
	if (cw->editor)
		answer->editor = mystrdup(cw->editor);
	if (cw->publisher)
		answer->publisher = mystrdup(cw->publisher);
	if (cw->date)
		answer->date = mystrdup(cw->date);
	if (cw->copyright)
		answer->copyright = mystrdup(cw->copyright);

	return answer;
}

void killcrossword(CROSSWORD *cw)
{
  if(cw)
  {
    free(cw->grid);
	free(cw->solution);
	free(cw->numbers);
	freelist(cw->wordsacross, cw->Nacross);
	freelist(cw->wordsdown, cw->Ndown);
	freelist(cw->acrossclues_grid, cw->width * cw->height);
	free(cw->cluesacross);
	freelist(cw->downclues_grid, cw->width * cw->height);
	free(cw->cluesdown);
	free(cw->numbersacross);
	free(cw->numbersdown);
	free(cw->title);
    free(cw->author);
    free(cw->editor);
    free(cw->publisher);
    free(cw->date);
	free(cw->copyright);
	free(cw);
  }
}

int crossword_setcell(CROSSWORD *cw, int cx, int cy, char ch)
{
  if(cx < 0 || cx >= cw->width || cy < 0 || cy >= cw->height)
    return -1;
  if(ch)
  {
    cw->solution[cy*cw->width+cx] = ch;
	cw->grid[cy*cw->width+cx] = 1;
  }
  else
    cw->grid[cy*cw->width+cx] = 0;

  freelist(cw->wordsacross, cw->Nacross);
  cw->wordsacross = getwordsacross(cw->solution, cw->grid, cw->width, cw->height, &cw->Nacross);
  freelist(cw->wordsdown, cw->Ndown);
  cw->wordsdown = getwordsdown(cw->solution, cw->grid, cw->width, cw->height, &cw->Ndown);
  renumber(cw);

  return 0;

}

int crossword_setsolutioncell(CROSSWORD *cw, int cx, int cy, char ch)
{
	int xstart = -1;
	int ystart = -1;
	int acrossnumber = 0;
	int downnumber = 0;
	int acrossindex = 0;
	int downindex = 0;
	int i;

	assert(cx >= 0 && cx < cw->width);
	assert(cy >= 0 && cy < cw->height);

	assert(cw->grid[cy * cw->width + cx] == 1);
	cw->solution[cy * cw->width + cx] = ch;

	if (acrosswordstart(cw->grid, cw->width, cw->height, cx, cy))
		xstart = cx;
	else if (cx > 0 && cw->grid[cy * cw->width + cx - 1] == 1)
	{
		xstart = cx - 1;
		while (!acrosswordstart(cw->grid, cw->width, cw->height, xstart, cy))
		{
			xstart--;
			if (xstart < 0)
				break;
		}
	}

	if (xstart != -1)
	{
		acrossnumber = cw->numbers[cy * cw->width + xstart];
		assert(acrossnumber > 0);
		for (i = 0; i < cw->Nacross; i++)
			if (cw->numbersacross[i] == acrossnumber)
				break;
		assert(i < cw->numbersacross);
		acrossindex = i;
		cw->wordsacross[acrossindex][cx - xstart] = ch;
	}

	if (downwordstart(cw->grid, cw->width, cw->height, cx, cy))
		ystart = cy;
	else if (cy > 0 && cw->grid[(cy - 1) * cw->width + cx] == 1)
	{
		ystart = cy - 1;
		while (!downwordstart(cw->grid, cw->width, cw->height, cx, ystart))
		{
			ystart--;
			if (ystart < 0)
				break;
		}
	}
	
	if (ystart != -1)
	{
		downnumber = cw->numbers[ystart * cw->width + cx];
		assert(downnumber > 0);
		for (i = 0; i < cw->Ndown; i++)
			if (cw->numbersdown[i] == downnumber)
				break;
		assert(i < cw->Ndown);
		downindex = i;
		cw->wordsdown[downindex][cy - ystart] = ch;
	}

	return 0;
}

int crossword_setacrossclue(CROSSWORD *cw, int id, const char *clue)
{
  int i, j;

  for(i=0;i<cw->width*cw->height;i++)
	  if(cw->numbers[i] == id)
	  {
		  free(cw->acrossclues_grid[i]);
		  cw->acrossclues_grid[i] = malloc(strlen(clue) + 1);
		  if (!cw->acrossclues_grid[i])
			  goto out_of_memory;
		  strcpy(cw->acrossclues_grid[i], clue);
		  for(j=0;j<cw->Nacross;j++)
			  if(cw->numbersacross[j] == id)
			  {
				  cw->cluesacross[j] = cw->acrossclues_grid[i];
		          break;
			  }
		  if(j == cw->Nacross)
            goto error_exit;
		  break;
	  }
  if(i == cw->width * cw->height)
     goto error_exit;
  return 0;
out_of_memory:
   return -1;
error_exit:
   return -2;
}

int crossword_setdownclue(CROSSWORD *cw, int id, const char *clue)
{
  int i, j;

  for(i=0;i<cw->width*cw->height;i++)
	  if(cw->numbers[i] == id)
	  {
		  free(cw->downclues_grid[i]);
		  cw->downclues_grid[i] = malloc(strlen(clue) + 1);
		  if (!cw->downclues_grid[i])
			  goto out_of_memory;
		  strcpy(cw->downclues_grid[i], clue);
		  for(j=0;j<cw->Ndown;j++)
			  if(cw->numbersdown[j] == id)
			  {
				  cw->cluesdown[j] = cw->downclues_grid[i];
		          break;
			  }
		  if(j == cw->Ndown)
            goto error_exit;
		  break;
	  }
  if(i == cw->width * cw->height)
     goto error_exit;
  return 0;
out_of_memory:
   return -1;
error_exit:
   return -2;
}

int crossword_resize(CROSSWORD *cw, int width, int height)
{
  char *solution = 0;
  unsigned char *grid = 0;
  int *numbers = 0;
  char **acrossclues_grid = 0;
  char **downclues_grid = 0;
  int i;
  int x, y;

  solution = malloc(width * height);
  if(!solution)
	  goto error_exit;
  grid = malloc(width * height);
  if(!grid)
	  goto error_exit;
  numbers = malloc(width * height * sizeof(int));
  if(!numbers)
    goto error_exit;
  memset(grid, 0, width * height);
  memset(solution, ' ', width * height);
  for(i=0;i<width*height;i++)
	  numbers[i] = 0;
  acrossclues_grid = malloc(width * height * sizeof(char *));
  if(!acrossclues_grid)
	  goto error_exit;
  for(i=0;i<width*height;i++)
	  acrossclues_grid[i] = 0;
  downclues_grid = malloc(width * height * sizeof(char *));
  if(!downclues_grid)
	  goto error_exit;
  for(i=0;i<width *height;i++)
	  downclues_grid[i] = 0;

  for(y=0;y<height && y < cw->height;y++)
    for(x =0; x < width && x < cw->width;x++)
	{
		solution[y*width+x] = cw->solution[y*cw->width+x];
		grid[y*width+x] = cw->grid[y*cw->width+x];
		acrossclues_grid[y*width+x] = cw->acrossclues_grid[y*cw->width+x];
		downclues_grid[y*width+x] = cw->downclues_grid[y*cw->width+x];
	}
  for(y=0;y<cw->height;y++)
    for(x=0;x<cw->width;x++)
	{
	  if(y >= height || x >= width)
	  {
		  free(cw->acrossclues_grid[y*cw->width+x]);
		  free(cw->downclues_grid[y*cw->width+x]);
	  }
	}
  free(cw->solution);
  free(cw->grid);
  free(cw->numbers);
  free(cw->acrossclues_grid);
  free(cw->downclues_grid);

  cw->solution = solution;
  cw->grid = grid;
  cw->numbers = numbers;
  cw->acrossclues_grid = acrossclues_grid;
  cw->downclues_grid = downclues_grid;

  cw->width = width;
  cw->height = height;

  freelist(cw->wordsacross, cw->Nacross);
  cw->wordsacross = getwordsacross(cw->solution, cw->grid, cw->width, cw->height, &cw->Nacross);
  if(cw->Nacross && cw->wordsacross == 0)
	  goto error_exit;
  freelist(cw->wordsdown, cw->Ndown);
  cw->wordsdown = getwordsdown(cw->solution, cw->grid, cw->width, cw->height, &cw->Ndown);
  if(cw->Ndown && cw->wordsdown == 0)
	  goto error_exit;
  renumber(cw);

  return 0;

error_exit:
  free(solution);
  free(grid);
  free(numbers);
  free(acrossclues_grid);
  free(downclues_grid);

  return -1;
}

void crossword_startgrid(CROSSWORD *cw)
{
  int i, ii;

  for(i=0;i<cw->height;i++)
    for(ii=0;ii<cw->width;ii++)
	{
	  crossword_setcell(cw, ii, i, 0);
	  if( (i % 2) == 0 || (ii % 2) == 0)
	  {
	    if(cw->grid[i*cw->width+ii] == 0)
		  crossword_setcell(cw, ii, i, ' ');
	  }
	}
}

void crossword_randgrid(CROSSWORD *cw)
{
  int i;
  int tx, ty;
  int Ntwo, Nmax;
  
   crossword_startgrid(cw);
   if (cw->width <= 4 || cw->height <= 4)
	   return;
   
   do
   {
	   Ntwo = 0;
	   Nmax = cw->width + cw->height;
		tx = rand() % cw->width;
		ty = rand() % cw->height;
		crossword_setcell(cw, tx, ty, 0);
		crossword_setcell(cw, cw->width - tx - 1, cw->height - ty - 1, 0);
		if (!crossword_connected(cw))
		{
			crossword_setcell(cw, tx, ty, ' ');
			crossword_setcell(cw, cw->width - tx - 1, cw->height - ty - 1, ' ');
			continue;
		}
	
		Nmax = 0;
		for (i = 0; i < cw->Nacross; i++)
		{
			if (strlen(cw->wordsacross[i]) == 2)
				Ntwo++;
			if (strlen(cw->wordsacross[i]) == cw->width)
				Nmax++;
		}
		for (i=0; i < cw->Ndown; i++)
		{
			if (strlen(cw->wordsdown[i]) == 2)
				 Ntwo++;
			if (strlen(cw->wordsdown[i]) == cw->height)
				   Nmax++;
		}
		if (Ntwo)
		{
			Nmax = cw->width + cw->height;
			crossword_setcell(cw, tx, ty, ' ');
			crossword_setcell(cw, cw->width - tx - 1, cw->height - ty - 1, ' ');
			continue;
		}

   } while (Nmax > 2);
 
}

int crossword_connected(CROSSWORD *cw)
{
	unsigned char *grid;
	int i, ii;
	int flag;


	grid = malloc(cw->width * cw->height);
	if (!grid)
		return 0;
	for (i = 0; i < cw->height;i++)
		for (ii = 0; ii < cw->width; ii++)
			grid[i*cw->width + ii] = cw->grid[i*cw->width + ii] ? 1 : 0;

	for (i = 0; i < cw->width*cw->height;i++)
		if (grid[i])
			break;
	if (i == cw->width*cw->height)
			return 0;
	grid[i] = 2;

	flag = 1;
	while (flag)
	{
		flag = 0;
		for (i = 0; i < cw->height; i++)
		{
			for (ii = 0; ii < cw->width; ii++)
			{
				if (grid[i*cw->width + ii] == 1)
				{
					if (i > 0 && grid[(i - 1)*cw->width + ii] == 2)
					{
						grid[i*cw->width + ii] = 2;
						flag = 1;
					}
					if (i < cw->height - 1 && grid[(i + 1)*cw->width + ii] == 2)
					{
						grid[i*cw->width + ii] = 2;
						flag = 1;
					}
					if (ii > 0 && grid[i*cw->width + ii - 1] == 2)
					{
						grid[i*cw->width + ii] = 2;
						flag = 1;
					}
					if (ii < cw->width - 1 && grid[i*cw->width + ii + 1] == 2)
					{
						grid[i*cw->width + ii] = 2;
						flag = 1;
					}
				}
			}
		}
	}

	for (i = 0; i < cw->width*cw->height;i++)
		if (grid[i] == 1)
			break;
	free(grid);
	if (i == cw->width *cw->height)
		return 1;
	return 0;
}

int crossword_gridsidentical(CROSSWORD* cwa, CROSSWORD* cwb)
{
	int x, y;

	if (cwa->width != cwb->width)
		return 0;
	if (cwa->height != cwb->height)
		return 0;
	if (memcmp(cwa->grid, cwb->grid, cwa->width * cwa->height))
		return 0;
	for (y = 0; y <cwa->height; y++)
		for (x = 0; x < cwa->width; x++)
		{
			if (cwa->grid[y * cwa->width + x] &&
				cwa->solution[y * cwa->width + x] != cwb->solution[y * cwb->width + x])
				return 0;
		}

	return 1;
}

static int renumber(CROSSWORD *cw)
{
  void *temp;
  int Ni = 0;
  int Nx = 0;
  int Ny = 0;
  int xstart, ystart;
  int x, y;

  temp = realloc(cw->numbersacross, cw->Nacross * sizeof(int));
  if(cw->Nacross > 0 && !temp)
    goto out_of_memory;
  cw->numbersacross = temp;

  temp = realloc(cw->numbersdown, cw->Ndown * sizeof(int));
  if(cw->Ndown > 0 && !temp)
	  goto out_of_memory;
  cw->numbersdown = temp;

  temp = realloc(cw->cluesacross, cw->Nacross * sizeof(char *));
  if(cw->Nacross > 0 && !temp)
	  goto out_of_memory;
  cw->cluesacross = temp;

  temp = realloc(cw->cluesdown, cw->Ndown * sizeof(char *));
  if(cw->Ndown > 0 && !temp)
	  goto out_of_memory;
  cw->cluesdown = temp;

  for(y=0;y<cw->height;y++)
    for(x=0;x<cw->width;x++)
	{
	  ystart = downwordstart(cw->grid, cw->width, cw->height, x, y);
	  xstart = acrosswordstart(cw->grid, cw->width, cw->height, x, y);
	  if(ystart == 0 && xstart == 0)
		  cw->numbers[y*cw->width+x] = 0;
	  else if(ystart == 1 && xstart == 0)
	  {
		  assert(Ny < cw->Ndown);
		  cw->numbers[y*cw->width+x] = Ni + 1;
		  cw->numbersdown[Ny] = Ni + 1;
		  cw->cluesdown[Ny] = cw->downclues_grid[y*cw->width+x];
		  Ny++;
		  Ni++;
	  }
	  else if(ystart == 0 && xstart == 1)
	  {
		  assert(Nx < cw->Nacross);
		  cw->numbers[y*cw->width+x] = Ni +1;
		  cw->numbersacross[Nx] = Ni + 1;
		  cw->cluesacross[Nx] = cw->acrossclues_grid[y*cw->width+x];
		  Nx++;
		  Ni++;
	  }
	  else if(ystart == 1 && xstart == 1)
	  {
		  assert(Ny < cw->Ndown);
		  assert(Nx < cw->Nacross);
		  cw->numbers[y*cw->width+x] = Ni + 1;
		  cw->numbersdown[Ny] = Ni + 1;
		  cw->cluesdown[Ny] = cw->downclues_grid[y*cw->width+x];
		  cw->numbersacross[Nx] = Ni + 1;
		  cw->cluesacross[Nx] = cw->acrossclues_grid[y*cw->width+x];
		  Ny++;
		  Nx++;
		  Ni++;
	  }
	}

	return 0;
out_of_memory:
	return -1;
}

static char **getwordsacross(char *solution, unsigned char *grid, int width, int height, int *N)
{
  int x, y;
  int i;
  char **answer = 0;
  void *temp;
  int Nfound = 0;
  int len;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	{
	  if(acrosswordstart(grid, width, height, x, y))
	  {
	    for(i=x;i<width && grid[y*width+i]==1;i++)
			    ;
		len = i - x;
	    temp = realloc(answer, (Nfound + 1) * sizeof(char *));
        if(!temp)
	      goto out_of_memory;
	    answer = temp;
	    answer[Nfound] = malloc(len +1);
	    if(!answer[Nfound])
		  goto out_of_memory;
		for(i=0;i<len;i++)
		  answer[Nfound][i] = solution[y*width+x+i];
	    answer[Nfound][len] = 0;
		Nfound++;
	  }
	}
	*N = Nfound;
	return answer;
out_of_memory:
	*N = -1;
    for(i=0;i<Nfound;i++)
	  free(answer[i]);
	free(answer);
    return 0;
}

static char **getwordsdown(char *solution, unsigned char *grid, int width, int height, int *N)
{
  int x, y;
  int i;
  char **answer = 0;
  void *temp;
  int Nfound = 0;
  int len;

  for(y=0;y<height;y++)
    for(x=0;x<width;x++)
	{
	  if(downwordstart(grid, width, height, x, y))
	  {
		for(i=y;i<height && grid[i*width+x]==1;i++)
			    ;
        len = i - y;
        temp = realloc(answer, (Nfound + 1) * sizeof(char *));
        if(!temp)
	      goto out_of_memory;
		answer = temp;
	    answer[Nfound] = malloc(len + 1);
	    if(!answer[Nfound])
          goto out_of_memory;
	    for(i=0;i<len;i++)
	      answer[Nfound][i] = solution[(y+i)*width+x];
	    answer[Nfound][len] = 0;
		Nfound++;
	  }
	}
	*N = Nfound;
	return answer;
out_of_memory:
	*N = -1;
    for(i=0;i<Nfound;i++)
	  free(answer[i]);
	free(answer);
    return 0;
}

static int downwordstart(unsigned char *grid, int width, int height, int x, int y)
{
  if(grid[y*width+x] == 1 && 
	  (y ==0 || grid[(y-1)*width+x] == 0) &&
	  y < height-1 && grid[(y+1)*width+x] == 1)
	    return 1;
  return 0;
}

static int acrosswordstart(unsigned char *grid, int width, int height, int x, int y)
{
  if(grid[y*width+x] == 1 && 
	  (x ==0 || grid[y*width+x-1] == 0) &&
	  x < width-1 && grid[y*width+x+1] == 1)
	    return 1;
  return 0;
}

static void freelist(char **str, int N)
{
  int i;

  if(str)
  {
    for(i=0;i<N;i++)
      free(str[i]);
    free(str);
  }
} 

static char *mystrdup(const char *str)
{
	char *answer;

	answer = malloc(strlen(str) + 1);
	if (answer)
		strcpy(answer, str);

	return answer;
}