#ifndef __WADFILE__
#define __WADFILE__

typedef struct {
	FILE		*handle;
	char		*pathname;
	STORAGE		*info;
	boolean		dirty;
} WADFILE;

void initFromFile(void);
void initNew(void);
void WadfileClose(void);
void WadfileFree(void);

int numLumps(void);
int lumpsize(int);
int lumpstart(int);

char const *lumpname(int);
int lumpNamed(char const *);

void *loadLump(int);
void *loadLumpNamed(char const *);

void addName(char const *, void *, int);
void writeDirectory(void);

#endif
