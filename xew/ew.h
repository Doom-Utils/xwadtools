/* ew.h ***************************************** updated: 1996-12-18 15:44 TT
 *
 * Description : Header file for ew.h
 * Author      : Tormod Tjaberg
 *               Changed by Hans Peter Verne, to incorporate
 *               definitions etc. for xforms-version.
 * Setup correct defines for each compiler...
 */

/* Try to determine the platform, default to MS_DOS */
#if !(defined(MS_DOS) ^ defined(UNIX))
#define MS_DOS
#include <dos.h>
#include <bios.h>
#endif

#ifdef __DJGPP__
#include <sys/nearptr.h>
#endif

#if defined(__WATCOMC__) || defined(__DJGPP__)
#include <signal.h>
#else
#include <sys/signal.h>
#endif

/* Compatibility section 
 *
 * Int4 should be a 32 bit (4 bytes) integer type. 
 * Uint4 should be a 32 bit (4 bytes) unsigned integer type 
 * If your int's are 16 bit, use long. 
 * If your long's are 64 bit (alpha), use int.  
 */
#ifdef MS_DOS
typedef long Int4;
typedef unsigned long Uint4;
#else
typedef int Int4;
typedef unsigned int Uint4;
#endif

/* The code below is to make things work with both the 3.02 and 3.10 
 * Zortech compiler it works under 3.02 but not under DPMI
 */
#if defined(DOS386)
#if defined(__ZTC__) && __ZTC__ <= 0x302
/* Versions prior to this have a hard time with int86 by itself */
#define int86  int86_real
/* Hardwire the address of DGROUP this will NOT work under DPMI */
Uint4 _x386_zero_base_ptr = 0xf0000000;
#else
/* To make the segment wrap work, you first have to find the base address
 * of DGROUP.  This varies depending on DPMI vs non DPMI.  
 */
extern void * _x386_zero_base_ptr;
#endif
#endif

/* Watcom has it's own way of doing everything, argh */
#if defined(__WATCOMC__) && defined(__386__)
#define int86  int386
#define WREGS(v,r) (v##.w.##r)
#else
#define WREGS(v,r) (v##.x.##r)
#endif

#ifdef UNIX
#if !(defined(CURSE) ^ defined(NCURSE) ^ defined(WITH_XFORMS) )
#error CURSE or NCURSE must be defined, or WITH_XFORMS ...
#endif

/* We hope for rudimentary support for arrow-keys. I'm certainly no 
 * expert on curses, so if this doesn't work for your system you
 * are more than welcome to fix it.  (hpv)
 */

#ifdef NCURSE
#include <ncurses.h>
#endif

#ifdef CURSE
#include <curses.h> 
#include <term.h> 
#endif

#define KEY_SPAWN   '\n'

#else   /* ie. msdos.  The following keys are otherwise defined by curses. */
#define KEY_DOWN    0x5000
#define KEY_UP      0x4800
#define KEY_HOME    0x4700 
#define KEY_END     0x4f00 
#define KEY_NPAGE   0x5100   /* Next page PgDn */
#define KEY_PPAGE   0x4900   /* Previous page PgUp */
#define KEY_SPAWN   '\r'     /* MS_DOS treats \n as two separate chars */
#endif

/* These keys are NOT defined by curses or anything, I think :-) */
#define KEY_ESC     0x001b
#define KEY_SPACE   ' '
#define KEY_EXPORT  'e'

/* These keys are defined just in case... (they are the numpad keys) */
#define KEY_X_DOWN    '2'
#define KEY_X_UP      '8'
#define KEY_X_HOME    '7'   
#define KEY_X_END     '1'   
#define KEY_X_NPAGE   '3'   /* PgDn */
#define KEY_X_PPAGE   '9'   /* PgUp */

/*********************/
/* various defines:  */
/*********************/
#define MAX_IMG_SIZ  (UINT_MAX - 32)   /* The maximum malloc can handle 
                                        * minus a little overhead
                                        */
#define SCR_WIDTH  320U
#define SCR_HEIGHT 200U

/* The bitmasks of the wad types must be mutually exclusive */
#define TAG_NONE   0    /* Not an image or mus or sfx */
#define TAG_FLAT   1    /* A flat, texture whatever */
#define TAG_FULL   2    /* Raw full screen 320*200 256 colours */
#define TAG_IMG    4    /* A graphic image in the "format" */
#define TAG_HIRES  8    /* A 16 bit splash screen 640*480 16 colours */
#define TAG_MUS    16   /* A MUS file */
#define TAG_SFX    32   /* A sound effect */

#define TAG_GFX    (TAG_FLAT | TAG_FULL | TAG_IMG | TAG_HIRES)
#define TAG_ALL    (TAG_GFX | TAG_MUS | TAG_SFX)

#define NUM_TAG    6     /* The number of different TAG types - NONE */


/* Abstract data types 
 */
typedef struct
{
   Int4 start;    /* offset to start of data */
   Int4 size;     /* byte size of data */
   char name[8];  /* name of data block */
} Directory;


typedef struct 
{
   char maker;
   char version;
   char code;
   char bpp;
   short x1;
   short y1;
   short x2;
   short y2;
   short hres;
   short vres;
   char palette[48]; /* palette */
   char vmode; 
   char nplanes;
   short bpl;
   short palinfo;
   char unused[128-70];
} PCXHDR;


/* Function prototypes : */
#include <stdio.h>           /* Needed for FILE  */
extern char *Get_pSearchList(char *);
extern char *FindFile(char *, char *);
extern void ReadIniFile(char *);
extern void ReadPal(char *, char *);
extern void BuildWadDir(FILE *, Directory *, unsigned char *, Int4);
extern unsigned char MapBitMaskToInx(unsigned char);
extern void Abort(char *, ...);
extern void CheckHost(void);
extern void GetArgs(int, char *[]);
extern void SigHandler(int);
extern void Beep(void);
extern void GenFileName(char *, char *, char *);
extern char Gen_WAV_File(FILE *, Directory, char *);
extern char Gen_MUS_File(FILE *, Directory, char *);
extern unsigned char *ParseImgBlock(Directory, unsigned char, 
		    short *, short *, short *, short *, FILE *);
extern short PCX_WriteImage(Directory, FILE *, short, short, 
			    unsigned char *, short, char *);

/* These are for the forms version... */

/* Declarations of global variables : */
extern char IniFileName[];
extern char TmpNameStr[];
extern char ExeStr[];

/* 256 RGB triplets */
extern char DAC_Reg[];             /* Current RGB */
extern const char doom_rgb[];      /* default RGB for DOOM */
extern const char heretic_rgb[];   /* default RGB for Heretic */
extern const char E_HERETIC_rgb[]; /* extra RGB for Heretic */
extern const char hexen_rgb[];     /* default RGB for Hexen */
extern const char strife_rgb[];    /* default RGB for Strife */

extern char Ext_WAV_Player[];
extern char Ext_MUS_Player[];
extern char Ext_PCX_Viewer[];
extern char *pPalFileSpec;
extern char *pWadFileSpec;
extern unsigned char *pBitMap;


void Message( char *, ... );  /* write a message in the message-browser. */
void SpinnWheel( int );

/* xpm conversion routines : */
extern void CreateXpmPalette( const char *, int );
extern char **CreateXpmImage( unsigned char *, int, int, int, int *);
extern int  CreateXpmFile( char **, const char *);

