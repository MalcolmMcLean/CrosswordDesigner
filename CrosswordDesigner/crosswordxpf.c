#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmlparser.h"
#include "crossword.h"

int saveasxpf(CROSSWORD *cw, char *fname);
int fsaveasxpf(CROSSWORD *cw, FILE *fp);
CROSSWORD **loadfromxpf(char *fname, int *N, int *err);


static CROSSWORD *crosswordfromnode(XMLNODE *node);
static char *mystrdupx(const char *str);

int saveasxpf(CROSSWORD *cw, char *fname)
{
  FILE *fp;

  fp = fopen(fname, "w");
  if(!fp)
    return -1;
  fprintf(fp, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
  fprintf(fp, "<!-- Conforms to Universal Crossword Puzzle Format XPF Version 1.0 -->\n");
  fprintf(fp, "<Puzzles Version=\"1.0\">\n");
  fsaveasxpf(cw, fp);
  fprintf(fp, "</Puzzles>\n");
  fclose(fp);

  return 0;
}

int fsaveasxpf(CROSSWORD *cw, FILE *fp)
{
  int i, ii;
  int row, col;

  fprintf(fp, "<Puzzle>\n");
  fprintf(fp, "<Type>normal</Type>\n");

  if(cw->title)
    fprintf(fp, "<Title>%s</Title>\n", cw->title);
  if(cw->author)
    fprintf(fp, "<Author>%s</Author>\n", cw->author);
  if(cw->editor)
    fprintf(fp, "<Editor>%s</Editor>\n", cw->editor);
  if(cw->copyright)
	  fprintf(fp, "<Copyright>%s</Copyright>\n", cw->copyright);
  if(cw->publisher)
	  fprintf(fp, "<Publisher>%s</Publisher>\n", cw->publisher);
  if(cw->date)
	  fprintf(fp, "<Date>%s</Date>\n", cw->date);
 
  fprintf(fp, "<Size>\n");
  fprintf(fp, "<Rows>%d</Rows>\n", cw->height);
  fprintf(fp, "<Cols>%d</Cols>\n", cw->width);
  fprintf(fp, "</Size>\n");
   
  fprintf(fp, "<Grid>\n");
  for(i=0;i<cw->height;i++)
  {
    fprintf(fp, "<Row>");
	for(ii=0;ii<cw->width;ii++)
		fprintf(fp, "%c", cw->grid[i*cw->width+ii] ? 
		cw->solution[i*cw->width+ii] : '.');
	fprintf(fp, "</Row>\n");
  }
  fprintf(fp, "</Grid>\n");

  fprintf(fp, "<Clues>\n");
  for(i=0;i<cw->Nacross;i++)
  {
    for(ii=0;ii<cw->width*cw->height;ii++)
		if(cw->numbers[ii] == cw->numbersacross[i])
			break;
	row = ii / cw->width + 1;
	col = (ii % cw->width) + 1;
    fprintf(fp, "<Clue Row = \"%d\" Col = \"%d\" Num = \"%d\" Dir = \"Across\" Ans = \"%s\">%s</Clue>\n",
	row, col, cw->numbersacross[i], cw->wordsacross[i], cw->cluesacross[i]);
  }
  for(i=0;i<cw->Ndown;i++)
  {
    for(ii=0;ii<cw->width*cw->height;ii++)
		if(cw->numbers[ii] == cw->numbersacross[i])
			break;
	row = ii / cw->width + 1;
	col = (ii % cw->width) + 1;
    fprintf(fp, "<Clue Row = \"%d\" Col = \"%d\" Num = \"%d\" Dir = \"Down\" Ans = \"%s\">%s</Clue>\n",
	row, col, cw->numbersdown[i], cw->wordsdown[i], cw->cluesdown[i]);
  }
  fprintf(fp, "</Clues>\n");
  fprintf(fp, "</Puzzle>\n");

  return 0;

}

#include <assert.h>

CROSSWORD **loadfromxpf(char *fname, int *N, int *err)
{
	XMLDOC *doc;
    XMLNODE *root;
    XMLNODE **puzzle;
    int errtype;
	int i, j;
	CROSSWORD **answer = 0;
	int Nfound;


	doc = loadxmldoc(fname, &errtype);
    if(!doc)
	  return 0;

	root = xml_getroot(doc);
	puzzle = xml_getdescendants(root, "Puzzle", &Nfound);
	if(Nfound > 0)
	{
	  answer = malloc(Nfound * sizeof(CROSSWORD *));
	  if(!answer)
	    goto out_of_memory;
	  j = 0;
	  for(i=0;i<Nfound;i++)
	  {
        answer[j] = crosswordfromnode(puzzle[i]);
	    if(answer[j])
		  j++;
	  }
	  if(j == 0)
	  {
	    free(answer);
		answer = 0;
	  }
	  *N = j;
	}
	free(puzzle);
	killxmldoc(doc);

	return answer;
out_of_memory:
	if(err)
	  *err = -1;
	free(puzzle);
    killxmldoc(doc);
	return 0;
}

static CROSSWORD *crosswordfromnode(XMLNODE *node)
{
  XMLNODE *child;
  XMLNODE *rownode;
  XMLNODE *colnode;
  XMLNODE *cluenode;
  CROSSWORD *answer = 0;
  char *solution = 0;
  const char *rowstr;
  const char *colstr;
  const char *numstr;
  const char *dirstr;
  const char *clue;
  int num;
  int width, height;
  int Nrows;
  int Nclues;
  int i, ii;

  child = xml_getchild(node, "Size", 0);
  if(!child)
    goto error_exit;

  rownode = xml_getchild(child, "Rows", 0);
  if(!rownode)
    goto error_exit;
  colnode = xml_getchild(child, "Cols", 0);
  if(!colnode)
    goto error_exit;
  rowstr = xml_getdata(rownode);
  colstr = xml_getdata(colnode);
  if(!rowstr || !colstr)
    goto error_exit;
  height = strtol(rowstr, 0, 10);
  width = strtol(colstr, 0, 10);
  if(height < 1 || width < 1)
    goto error_exit;
  if( (height * width)/width != height)
    goto error_exit;

  solution = malloc(width * height);
  if(!solution)
    goto error_exit;
  memset(solution, 0, width * height);
  child = xml_getchild(node, "Grid", 0);
  Nrows = xml_Nchildrenwithtag(child, "Row");
  if(Nrows != height)
    goto error_exit;
  for(i=0;i<height;i++)
  {
    rownode = xml_getchild(child, "Row", i);
	if(!rownode)
	  goto error_exit;
	rowstr = xml_getdata(rownode);
	if(!rowstr)
	  goto error_exit;
	if(strlen(rowstr) != width)
	  goto error_exit;
	for(ii=0;ii<width;ii++)
	  solution[i*width+ii] = rowstr[ii];
  }
  answer = createcrossword(width, height);
  if(!answer)
    goto error_exit;
  for(i=0;i<height;i++)
    for(ii=0;ii<width;ii++)
		crossword_setcell(answer, ii, i, solution[i*width+ii] == '.' ? 0 : solution[i*width+ii] );

  child = xml_getchild(node, "Clues", 0);
  if(child)
  {
    Nclues = xml_Nchildrenwithtag(child, "Clue");
    for(i=0;i<Nclues;i++)
    {
      cluenode = xml_getchild(child, "Clue", i);
	  dirstr = xml_getattribute(cluenode, "Dir");
	  numstr = xml_getattribute(cluenode, "Num");
	  clue = xml_getdata(cluenode);
	  num = strtol(numstr, 0, 10);
	  if(!strcmp(dirstr, "Across"))
	    crossword_setacrossclue(answer, num, clue);
	  else if(!strcmp(dirstr, "Down"))
	    crossword_setdownclue(answer, num, clue);
	}
  }
  free(solution);

  child = xml_getchild(node, "Title", 0);
  if(child)
	  answer->title = mystrdupx(xml_getdata(child));
  child = xml_getchild(node, "Author", 0);
  if(child)
	  answer->author = mystrdupx(xml_getdata(child));
  child = xml_getchild(node, "Editor", 0);
  if(child)
	  answer->editor = mystrdupx(xml_getdata(child));
  child = xml_getchild(node, "Copyright", 0);
  if(child)
	  answer->copyright = mystrdupx(xml_getdata(child));
  child = xml_getchild(node, "Publisher", 0);
  if(child)
	  answer->publisher = mystrdupx(xml_getdata(child));
  child = xml_getchild(node, "Date", 0);
  if(child)
	  answer->date = mystrdupx(xml_getdata(child));

  return answer;

error_exit:
  free(solution);
  killcrossword(answer);
  return 0;
}

/*
  strdup - returns null if passed null
*/
static char *mystrdupx(const char *str)
{
  char *answer = 0;

  if(str)
  {
    answer = malloc(strlen(str) + 1);
	if(answer)
      strcpy(answer, str);
  }

  return answer;
}