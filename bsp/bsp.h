/*- BSP.H ------------------------------------------------------------------*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <assert.h>

#if defined(MSDOS) || defined(__MSDOS__)
#include <dos.h>
#endif
#ifdef __TURBOC__
#include <alloc.h>
#endif

/*- boolean constants ------------------------------------------------------*/

#define TRUE			1
#define FALSE			0

/*- The function prototypes ------------------------------------------------*/

static void ProgError( char *, ...);
static void *GetMemory(size_t);
static void *ResizeMemory(void *, size_t);
static unsigned ComputeAngle(long,long);

#undef max
#define max(a,b) (((a)>(b))?(a):(b))

/*- print a resource name by printing first 8 characters --*/

#define Printname(dir) printf("%-8.8s",(dir)->name)

/*------------------------------- end of file ------------------------------*/
