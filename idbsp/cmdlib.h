/*
	CMDLIB.H
*/

#ifndef __CMDLIB__
#define __CMDLIB__

void *SafeMalloc(unsigned);
void *SafeCalloc(unsigned, unsigned);
void *SafeRealloc(void *, unsigned);
void Error(char *error, ...);

#endif
