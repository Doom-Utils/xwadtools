/*
 *	cawf.h - definitions for cawf(1)
 */

/*
 *	Copyright (c) 1991 Purdue University Research Foundation,
 *	West Lafayette, Indiana 47907.  All rights reserved.
 *
 *	Written by Victor A. Abell <abe@mace.cc.purdue.edu>,  Purdue
 *	University Computing Center.  Not derived from licensed software;
 *	derived from awf(1) by Henry Spencer of the University of Toronto.
 *
 *	Permission is granted to anyone to use this software for any
 *	purpose on any computer system, and to alter it and redistribute
 *	it freely, subject to the following restrictions:
 *
 *	1. The author is not responsible for any consequences of use of
 *	   this software, even if they arise from flaws in it.
 *
 *	2. The origin of this software must not be misrepresented, either
 *	   by explicit claim or by omission.  Credits must appear in the
 *	   documentation.
 *
 *	3. Altered versions must be plainly marked as such, and must not
 *	   be misrepresented as being the original software.  Credits must
 *	   appear in the documentation.
 *
 *	4. This notice may not be removed or altered.
 */

#include <stdio.h>
#ifdef	STDLIB
#include <stdlib.h>
#endif
#ifdef	UNIX
#include <sys/types.h>
#else
#include <sys\types.h>
#endif
#include "regexp.h"

#if defined(UNIX) && !defined(__CYGWIN32__)
#define CAWFLIB         "/usr/local/lib/cawf"	/* UNIX library location */
#else
#define CAWFLIB         "c:/lib/cawflib"	/* PC-DOS library location */
#endif
						/* (CAWFLIB environment
						 * variable over-rides it) */
#define MAXEXP          30			/* maximum expressions
						 * (and TABs) */
#define MAXFILES        10			/* maximum files (including
						 * *.dev, common and *.mac) */
#define MAXFSTK		5			/* maximum file sttack
						 * (for .so) */
#define MAXHYCH		10			/* maximum hyphen characters */
#define MAXLINE         256			/* maximum line length */
#define MAXMACRO        100			/* maximum number of macros */
#define MAXMTXT         1024			/* maximum macro text lines */
#define MAXNHNR		10			/* maximum ".NH" numbers
						 * (but 0 not used) */
#define MAXNR		50			/* maximum number reg */
#define MAXOLL		512			/* maximum output line length */
#define	MAXSCH		256			/* maximum special characters */
#define MAXSP		25			/* maximum stack pointer (for
						 * nesting of macro calls) */
#define MAXSTR		100			/* maximum ".ds" strings */

/*
 * Output line adjustment modes
 */

#define LEFTADJ		0
#define RIGHTADJ	1
#define BOTHADJ		2

/*
 * Error handling codes
 */

#define	FATAL		0			/* fatal error */
#define	LINE		0			/* display line */
#define	NOLINE		1			/* don't display line */
#define WARN		1			/* warning error */

/*
 * Padding directions
 */

#define	PADLEFT		0			/* pad from left */
#define PADRIGHT	1			/* pad from right */

/*
 * Pass 3 signal codes
 */

#define NOBREAK		-1
#define DOBREAK		-2
#define MESSAGE		-3

/*
 * Macro argument types
 */

#define	MANMACROS	1			/* -man */
#define MSMACROS	2			/* -ms */


struct fcode {
	char nm;			/* font name character */
	char status;			/* status */
};

struct hytab {
	char font;			/* font name character */
	int len;			/* effective length */
	char *str;			/* value string */
};

struct macro {
        char name[2];			/* macro name */
        int bx;				/* beginning Macrotxt[] index */
	int ct;				/* index count */
};

struct nbr {
	char nm[2];                     /* register name */
	int val;			/* value */
};

struct parms {
	char nm[2];                     /* parameter name */
	char *cmd;                      /* pass 3 command */
	int val;                        /* current value */
	int prev;                       /* previous value */
};

struct rx {
	char *re;			/* regular expression */
	struct regexp *pat;		/* compiled pattern */
};

struct scale {
	char nm;			/* scale factor name */
	double val;			/* value */
};

struct schtab {
	char nm[2];			/* character name */
	int len;			/* effective length */
	char *str;			/* value string */
};

struct str {
	char nm[2];			/* string name */
	char *str;			/* string value */
};

extern int Adj;				/* output line adjustment mode */
extern char *Aftnxt;			/* action after next line */
extern char *Args[];			/* macro arguments */
extern char *Argstack[];                /* stack for Expand()'s "args[]" */
extern int Backc;                       /* last word ended with '\\c' */
extern int Botmarg;			/* bottom margin */
extern int Centering;                   /* centering count */
extern int Condstack[];                 /* stack for Expand()'s "cond" */
extern char *Cont;			/* continue line append */
extern int Contlen;			/* continue line append length */
extern int Curmx;                 	/* current macro name */
extern FILE *Efs;			/* error file stream pointer */
extern char *Eol;			/* end of line information */
extern int Eollen;			/* end of line length */
extern int Err;                         /* error flag */
extern char *F;                         /* field value */
extern struct fcode Fcode[];		/* font codes */
extern int Fill;			/* fill status */
extern char Font[];                     /* current font */
extern int Fph;				/* first page header status */
extern int Fsp;                         /* files stack pointer (for .so) */
extern char *Ftc;                       /* center footer */
extern char *Ftl;                       /* left footer */
extern char *Ftr;                       /* right footer */
extern char *Hdc;			/* center header */
extern int Hdft;			/* header/footer status */
extern char *Hdl;			/* left header */
extern char *Hdr;			/* right header */
extern FILE *Ifs;			/* input file stream */
extern FILE *Ifs_stk[];			/* Ifs stack */
extern int Ind;                         /* indentation amount */
extern char *Inname;                    /* input file name */
extern char *Inn_stk[];			/* Inname stack */
extern struct hytab Hychar[];           /* hyphen characters */
extern int LL;				/* line length */
extern char Line[];                     /* input line */
extern int Lockil;			/* pass 2 line number is locked
					 * (processing is inside macro) */
extern int Marg;                        /* macro argument - man, ms, etc. */
extern struct macro Macrotab[];         /* macro table */
extern int Macrostack[];                /* stack for Expand()'s "macro" */
extern char *Macrotxt[];                /* macro text */
extern int Mtx;                         /* macro text index */
extern int Mxstack[];                   /* stack for Expand()'s "mx" */
extern int Nhnr[];			/* ".NH" numbers */
extern int Nhy;                         /* number of Hychar[] entries */
extern int Nleftstack[];                /* stack for Expand()'s "nleft" */
extern int Nmac;                        /* number of macros */
extern int Nnr;                         /* number of Numb[] entries */
extern int Nospmode;			/* no space mode */
extern int Nparms;                      /* number of Parms[] entries */
extern int NR;                          /* number of record, ala awk */
extern int NR_stk[];			/* NR stack */
extern int Nsch;                        /* number of Schar[] entries */
extern int Nstr;                        /* number of entries in Str[] */
extern int Ntabs;			/* number of TAB positions */
extern struct nbr Numb[];		/* number registers */
extern int Nxtln;			/* next line number */
extern int Outll;			/* output line length */
extern char Outln[];			/* output line */
extern int Outlx;			/* output line index */
extern int P2il;                        /* pass 2 input line number */
extern char *P2name;                    /* pass 2 input file name */
extern int P3fill;			/* pass 3 fill status */
extern int Padchar[];			/* padding character locations */
extern int Padfrom;			/* which end to pad from */
extern int Padx;			/* Padchar[] index */
extern struct parms Parms[];            /* parameter registers */
extern char Pass1ln[];                  /* pass 1 output line */
extern char Pass2ln[];                  /* pass 2 output line */
extern struct rx Pat[];			/* compiled regexp patterns */
extern int Pglen;			/* page length */
extern int Pgoff;			/* page offset */
extern char *Pname;                     /* program name */
extern char Prevfont;                   /* previous font */
extern int Ptrstack[];                  /* stack for Expand()'s "ptr" */
extern struct scale Scale[];		/* scaling factors */
extern double Scalen;                   /* 'n' scaling factor */
extern double Scaleu;                   /* 'u' scaling factor */
extern double Scalev;                   /* 'v' scaling factor */
extern struct schtab Schar[];           /* special characters */
extern int Sp;				/* stack pointer */
extern struct str Str[];		/* ".ds" strings */
extern int Sx;				/* string index */
extern int Tabs[];			/* TAB positions */
extern int Thispg;			/* this page number */
extern int Tind;			/* temporary indentation amount */
extern int Topmarg;			/* top margin */
extern int Uhyph;			/* hyphen usage state */
extern int Vspace;                      /* vertical (inter-text-line) spacing */
extern char Word[];			/* pass 2 word buffer */
extern int Wordl;                       /* effective length of Word[] */
extern int Wordx;                       /* Word[] index */

char *Asmcode(), *Field(), *Findstr(), *Newstr(), *New2str();
void  Error(), Macro(), Pass2(), Pass3();
regexp *Rxpat();

#ifndef	STDLIB
char *getenv(), *malloc(), *strchr(), *strrchr();
#endif
