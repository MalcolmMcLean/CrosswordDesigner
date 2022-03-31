#ifndef balnkgrids_h
#define blankgrids_h

typedef struct
{
	int width;
	int height;
	const char* grid; /* spaces and hashes, nul terminated */
} BLANKGRID;

char* randomamericanblankgrid(int* width, int* height);

#endif
