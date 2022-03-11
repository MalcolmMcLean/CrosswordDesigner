#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "crossword.h"

CROSSWORD *loadfrompuz(char *fname, int *err);
CROSSWORD *floadfrompuz(FILE *fp, int *err);
int saveaspuz(CROSSWORD *cw, char *fname);
int fsaveaspuz(CROSSWORD *cw, FILE *fp);


typedef struct
{
	int checksum;
	char *filemagic;
	int cib_checksum;
	unsigned char low_checksums[4];
	unsigned char high_checksums[4];
	char *version;
	int reserved;
	int scrambled_checksum;
	unsigned char reservedstr[12];
	int width;
	int height;
	int Nclues;
	int bitmask;
	int scrambled_tag;

} PUZHEADER;

static unsigned short checksum_file(CROSSWORD *cw, char **clues, int Nclues, char *solution, char *grid, char *title, char *author, char *copyright, char *notes);
static unsigned short checksum_partial(CROSSWORD *cw, char **clues, int Nclues, char *title, char *author, char *copyright, char *notes);
static unsigned short checksum_cib(CROSSWORD *cw);
static char **getclues(CROSSWORD *cw, int *N);
static char *getsolution(CROSSWORD *cw);
static char *getplayergrid(CROSSWORD *cw);
static unsigned short cksum_region(unsigned char *base, int len, unsigned short cksum);
static int floadpuzheader(PUZHEADER *hdr, FILE *fp);
static int fputasciiz(const char *str, FILE *fp);
static char *freadasciiz(FILE *fp);
static int fput16le(int x, FILE *fp);
static int fget16le(FILE *fp);
static char *mystrdup(const char *str);


CROSSWORD *loadfrompuz(char *fname, int *err)
{
	FILE *fp;
	CROSSWORD *answer;

	fp = fopen(fname, "rb");
	if (!fp)
	{
		if (err)
			*err = -2;
		return 0;
	}
	answer = floadfrompuz(fp, err);
	fclose(fp);
	return answer;
}

int saveaspuz(CROSSWORD *cw, char *fname)
{
	FILE *fp;
	int answer;

	fp = fopen(fname, "wb");
	if (!fp)
		return -1;
	fsaveaspuz(cw, fp);
	answer = ferror(fp);
	fclose(fp);
	return answer;
}

CROSSWORD *floadfrompuz(FILE *fp, int *err)
{
	int error = -1;
	CROSSWORD *answer = 0;
	char *board  = 0;
	char *player_state = 0;
	char *title = 0;
	char *author = 0;
	char *copyright = 0;
	int i;
	char **clues = 0;
	PUZHEADER header;
	int ch;
	int x, y;
	int ai, di;

	error = floadpuzheader(&header, fp);
	if (error)
		goto error_exit;
	error = -1;
	board = malloc(header.width * header.height + 1);
	if (!board)
		goto error_exit;
	fread(board, 1, header.width*header.height, fp);
	player_state = malloc(header.width * header.height +1);
	if (!player_state)
		goto error_exit;
	fread(player_state, 1, header.width * header.height, fp);
	title = freadasciiz(fp);
	author = freadasciiz(fp);
	copyright = freadasciiz(fp);
	clues = malloc(header.Nclues * sizeof(char *));
	if (!clues)
		goto error_exit;
	for (i = 0; i < header.Nclues; i++)
		clues[i] = freadasciiz(fp);

	if (feof(fp) || ferror(fp))
	{
		error = -3;
		goto error_exit;
	}
	answer = createcrossword(header.width, header.height);
	if (!answer)
		goto error_exit;

	for (y = 0; y < header.height; y++)
	{
		for (x = 0; x < header.width; x++)
		{
			ch = board[y*header.width + x];
			if (ch == '.')
				ch = 0;
			crossword_setcell(answer, x, y, ch);
		}
	}

	ai = 0;
	di = 0;
	i = 0;
	error = -3;
	for (y = 0; y < header.height; y++)
	{
		for (x = 0; x < header.width; x++)
		{
			if (answer->numbers[y*answer->width + x])
			{
				if (answer->numbersacross[ai] == answer->numbers[y*answer->width + x])
				{
					crossword_setacrossclue(answer, answer->numbers[y*answer->width + x], clues[i]);
					ai++;
					i++;
					if (i > header.Nclues)
						goto error_exit;
				}
				if (answer->numbersdown[di] == answer->numbers[y*answer->width + x])
				{
					crossword_setdownclue(answer, answer->numbers[y*answer->width + x], clues[i]);
					di++;
					i++;
					if (i > header.Nclues)
						goto error_exit;
				}
			}
		}
	}
	if (i != header.Nclues)
		goto error_exit;
	answer->copyright = copyright;
	answer->author = author;
	answer->title = title;

	free(board);
	free(player_state);
	for (i = 0; i < header.Nclues; i++)
		free(clues[i]);
	free(clues);
	*err = 0;
	return answer;
error_exit:
	*err = error;
	return 0;
}

int fsaveaspuz(CROSSWORD *cw, FILE *fp)
{
	int scrambled_checksum = 0;
	int checksum;
	int cib_checksum;
	int partial_checksum;
	int solution_checksum;
	int grid_checksum;
	unsigned char reserved_str[12] = "12345678901";
	unsigned char *mask = "ICHEATED";
	char *title;
	char *author;
	char *copyright;
	char *notes;
	char **clues = 0;
	int Nclues;
	char *board;
	char *player_grid;
	int bitmask = 0;
	int i;

	clues = getclues(cw, &Nclues);
	board = getsolution(cw);
	player_grid = getplayergrid(cw);
	title = cw->title ? cw->title : "Title";
	author = cw->author ? cw->author : "Author";
	copyright = cw->copyright ? cw->copyright : "(c) Copyright";
	notes = "Some notes";

	checksum = checksum_file(cw, clues, Nclues, board, player_grid, title, author, copyright, notes);
	cib_checksum = checksum_cib(cw);
	partial_checksum = checksum_partial(cw, clues, Nclues, title, author, copyright, notes);
	solution_checksum = cksum_region(board, cw->width * cw->height, 0);
	grid_checksum = cksum_region(player_grid, cw->width * cw->height, 0);


	fput16le(checksum, fp);
	fputasciiz("ACROSS&DOWN", fp);
	fput16le(cib_checksum, fp);
	fputc(mask[0] ^ (cib_checksum & 0xFF), fp);
	fputc(mask[1] ^ (solution_checksum & 0xFF), fp);
	fputc(mask[2] ^ (grid_checksum & 0xFF), fp);
	fputc(mask[3] ^ (partial_checksum & 0xFF), fp);

	fputc(mask[4] ^ ((cib_checksum >> 8) & 0xFF), fp);
	fputc(mask[5] ^ ((solution_checksum >> 8) & 0xFF), fp);
	fputc(mask[6] ^ ((grid_checksum >> 8) & 0xFF), fp);
	fputc(mask[7] ^ ((partial_checksum >> 8) & 0xFF), fp);

	fwrite("v1.2", 1, 4, fp);
	fput16le(0, fp); /* reserved */
	fput16le(scrambled_checksum, fp);
	fwrite(reserved_str, 1, 12, fp);
	fputc(cw->width, fp);
	fputc(cw->height, fp);
	fput16le(cw->Nacross + cw->Ndown, fp);
	fput16le(bitmask, fp);
	fput16le(0, fp); /* scrambled_tag */

	fwrite(board, 1, cw->width * cw->height, fp);
	fwrite(player_grid, 1, cw->width * cw->height, fp);
	fputasciiz(title, fp);
	fputasciiz(author, fp);
	fputasciiz(copyright, fp);
	for (i = 0; i < cw->Nacross + cw->Ndown; i++)
		fputasciiz(clues[i], fp);
	fputasciiz(notes, fp);

	free(board);
	free(player_grid);
	if (clues)
	{
		for (i = 0; i < Nclues; i++)
			free(clues[i]);
	}
	free(clues);
	return 0;
}

static unsigned short checksum_file(CROSSWORD *cw, char **clues, int Nclues, char *solution, char *grid, char *title, char *author, char *copyright, char *notes)
{
	unsigned short answer;
	int i;

	answer = checksum_cib(cw);
	answer = cksum_region(solution, cw->width *cw->height, answer);
	answer = cksum_region(grid, cw->width * cw->height, answer);
    answer = cksum_region(title, strlen(title) + 1, answer);
	answer = cksum_region(author, strlen(author) + 1, answer);
    answer = cksum_region(copyright, strlen(copyright) + 1, answer);

	for (i = 0; i < Nclues; i++)
		answer = cksum_region(clues[i], strlen(clues[i]), answer);

	answer = cksum_region(notes, strlen(notes) + 1, answer);

	return answer;
}

static unsigned short checksum_partial(CROSSWORD *cw, char **clues, int Nclues, char *title, char *author, char *copyright, char *notes)
{
	unsigned short answer = 0;
	int i;

	answer = cksum_region(title, strlen(title) +1, answer);
	answer = cksum_region(author, strlen(author) + 1, answer);
	answer = cksum_region(copyright, strlen(copyright) + 1, answer);
	for (i = 0; i < Nclues; i++)
		answer = cksum_region(clues[i], strlen(clues[i]), answer);
	answer = cksum_region(notes, strlen(notes) + 1, answer);

	return answer;
}

static unsigned short checksum_cib(CROSSWORD *cw)
{
	unsigned char buff[8];
	int Nclues;
	int bitmask;

	Nclues = cw->Nacross + cw->Ndown;
	bitmask = 0;

	buff[0] = cw->width;
	buff[1] = cw->height;
	buff[2] = Nclues & 0xFF;
	buff[3] = (Nclues >> 8) & 0xFF;
	buff[4] = bitmask & 0xFF;
	buff[5] = (bitmask >> 8) & 0xFF;
	buff[6] = 0;
	buff[7] = 0;

	return cksum_region(buff, 8, 0);

}

static char **getclues(CROSSWORD *cw, int *N)
{
	char **answer;
	int i;
	int ai = 0;
	int di = 0;

	answer = malloc((cw->Nacross + cw->Ndown) * sizeof(char *));
	for (i = 0; i < cw->Nacross + cw->Ndown; i++)
	{
		if (di == cw->Ndown || cw->numbersacross[ai] <= cw->numbersdown[di])
		{
			answer[i] = cw->cluesacross[ai] ? mystrdup(cw->cluesacross[ai]) : mystrdup("Clue");
			ai++;
		}
		else
		{
			answer[i] = cw->cluesdown[di] ? mystrdup(cw->cluesdown[di]) : mystrdup("Clue");
			di++;
		}
	}
	*N = cw->Nacross + cw->Ndown;
	return answer;
}

static char *getsolution(CROSSWORD *cw)
{
	char *answer;
	int x, y;

	answer = malloc(cw->width * cw->height + 1);
	if (!answer)
		return 0;
	for (y = 0; y < cw->height; y++)
	  for (x = 0; x < cw->width; x++)
		answer[y*cw->width + x] = cw->grid[y*cw->width + x] ? cw->solution[y*cw->width + x] : '.';
   
    answer[cw->width*cw->height] = 0;

	return answer;
}

static char *getplayergrid(CROSSWORD *cw)
{
	char *answer;
	int x, y;

	answer = malloc(cw->width * cw->height + 1);
	if (!answer)
		return 0;
	for (y = 0; y < cw->height; y++)
	for (x = 0; x < cw->width; x++)
		answer[y*cw->width + x] = cw->grid[y*cw->width + x] ? '-' : '.';
	answer[cw->width*cw->height] = 0;

	return answer;
}

/*
Checksum over 8 byes of board header, starting at width
c_cib = cksum_region(data + 0x2C, 8, 0);

cksum = c_cib;
cksum = cksum_region(solution, w*h, cksum);
cksum = cksum_region(grid, w*h, cksum);

if (strlen(title) > 0)
cksum = cksum_region(title, strlen(title)+1, cksum);

if (strlen(author) > 0)
cksum = cksum_region(author, strlen(author)+1, cksum);

if (strlen(copyright) > 0)
cksum = cksum_region(copyright, strlen(copyright)+1, cksum);

for(i = 0; i < num_of_clues; i++)
cksum = cksum_region(clue[i], strlen(clue[i]), cksum);

if (strlen(notes) > 0)
cksum = cksum_region(notes, strlen(notes)+1, cksum)
*/
static unsigned short cksum_region(unsigned char *base, int len, unsigned short cksum) 
{
	int i;

	for (i = 0; i < len; i++) {
		if (cksum & 0x0001)
			cksum = (cksum >> 1) + 0x8000;
		else
			cksum = cksum >> 1;
		cksum += *(base + i);
	}

	return cksum;
}

static int floadpuzheader(PUZHEADER *hdr, FILE *fp)
{
	hdr->checksum = fget16le(fp);
	hdr->filemagic = freadasciiz(fp);
	hdr->cib_checksum = fget16le(fp);;
	fread(hdr->low_checksums, 1, 4, fp);
	fread(hdr->high_checksums, 1, 4, fp);
	hdr->version = malloc(5);
	fread(hdr->version, 1, 4, fp);
	hdr->version[4] = 0;
	hdr->reserved = fget16le(fp);
	hdr->scrambled_checksum = fget16le(fp);
	fread(hdr->reservedstr, 1, 12, fp);
	hdr->width = fgetc(fp);
	hdr->height = fgetc(fp);
	hdr->Nclues = fget16le(fp);
	hdr->bitmask = fget16le(fp);
	hdr->scrambled_tag = fget16le(fp);
	if (feof(fp) || ferror(fp))
		return -3;
	return 0;
}

static void freepuzheader(PUZHEADER *hdr)
{
	free(hdr->filemagic);
	free(hdr->version);
}

static char *freadasciiz(FILE *fp)
{
	char *answer;
	char *temp;
	int capacity = 32;
	int N = 0;
	int ch;

	answer = malloc(capacity);
	if (!answer)
		goto out_of_memory;
	while ((ch = fgetc(fp)) != EOF)
	{
		if (N == capacity - 1)
		{
			temp = realloc(answer, capacity + capacity / 2);
			if (!temp)
				goto out_of_memory;
			answer = temp;
			capacity += capacity / 2;
		}
		answer[N++] = (char)ch;
		if (!ch)
			break;
	}
	if (ch)
		answer[N++] = 0;
	return answer;
out_of_memory:
	free(answer);
	return 0;
}

static int fputasciiz(const char *str, FILE *fp)
{
	while (*str)
	{
		fputc(*str, fp);
		str++;
	}
	fputc(0, fp);
	return 0;
}

static int fget16le(FILE *fp)
{
	int answer = 0;
	answer = fgetc(fp) | (fgetc(fp) << 8);
	return answer;
}

static int fput16le(int x, FILE *fp)
{
	fputc(x & 0xFF, fp);
	fputc((x >> 8) & 0xFF, fp);
	return 0;
}

static char *mystrdup(const char *str)
{
	char *answer;

	answer = malloc(strlen(str) + 1);
	if (answer)
		strcpy(answer, str);
	return answer;
}