/*
 *	cawf - a C version of Henry Spencer's awf(1), the Amazingly
 *	       Workable (text) Formatter
 *
 *	V. Abell, Purdue University Computing Center
 *	Version 1.0, February, 1991
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

#include "cawf.h"

#ifdef	UNIX
#include <string.h>
#include <sys/stat.h>
#else
#include <io.h>
#include <process.h>
#include <string.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif


main(argc, argv)
	int     argc;
	char    *argv[];
{
	int ax = 1;             	/* argument index */
	char buf[MAXLINE];		/* character buffer */
	char *ep;               	/* environment pointer */
	int err = 0;            	/* error count */
	char *files[MAXFILES];  	/* file names */
	int i;	               		/* temporary indexes */
	size_t l;                       /* length */
	char *lib = CAWFLIB;            /* library path */
	int libl;			/* library path length */
	int nf = 0;             	/* number of files */
	char *np;               	/* name pointer */
	int pc;                 	/* prolog count */
	struct stat sbuf;               /* stat buffer */
/*
 * Save program name.
 */
	if ((Pname = strrchr(argv[0], '\\')) != NULL)
		Pname++;
	else if ((Pname = strrchr(argv[0], '/')) != NULL)
		Pname++;
	else
		Pname = argv[0];
/*
 * Set error file stream pointer.
 */
	Efs = stderr;
/*
 * Get library name.
 */
	if ((np = getenv("CAWFLIB")) != NULL)
		lib = np;
	libl = strlen(lib);
/*
 * Get device file name.
 */
#if 0
	if ((ep = getenv("TERM")) == NULL || *ep == '\0')
#endif
		ep = "dumb";
	l = libl + 1 + strlen(ep) + strlen(".dev") + 1;
	if ((np = malloc(l)) == NULL)
		Error(FATAL, NOLINE, " no string space for device file name",
			NULL);
	(void) sprintf(np, "%s/%s.dev", lib, ep);
	files[nf++] = np;
/*
 * Get common text file name.
 */
	l = libl + 1 + strlen("common") + 1;
	if ((np = malloc(l)) == NULL)
		Error(FATAL, NOLINE, " no string space for common file name",
			NULL);
	(void) sprintf(np, "%s/common", lib);
	files[nf++] = np;
/*
 * Get macro text file name.
 */
	if (argc > 1) {
		if (strncmp(argv[1], "-m", 2) == 0) {
			l = libl + 1 + strlen(argv[1]+1) + strlen(".mac") + 1;
			if ((np = malloc(l)) == NULL)
				Error(FATAL, NOLINE, " no string space for ",
					argv[1]);
			(void) sprintf(np, "%s/%s.mac", lib, argv[1]+1);
			files[nf++] = np;
			if (strcmp(argv[1], "-man") == 0)
				Marg = MANMACROS;
			else if (strcmp(argv[1], "-ms") == 0)
				Marg = MSMACROS;
			ax = 2;
		} else {
		    /*
		     * No macroes - enable Bold, Italic and Roman fonts.
		     */
			for (i = 0; Fcode[i].nm; i++) {
				switch (Fcode[i].nm) {
				case 'B':
				case 'I':
				case 'R':
					Fcode[i].status = '1';
				}
			}
		}
	}
/*
 * Add user-supplied file names.
 */
	pc = nf;
	if (ax >= argc) {
		files[nf++] = NULL;       /* STDIN */
	} else {
		while (ax < argc) {
			if (nf >= MAXFILES)
				Error(WARN, NOLINE, " too many files at ",
					argv[ax]);
			files[nf++] = argv[ax++];
		}
	}
/*
 * Make sure all input files are accessible.
 */
	for (i = 0; i < nf; i++) {
		if (files[i] != NULL) {
			if (stat(files[i], &sbuf) != 0)
				Error(WARN, NOLINE, " can't find ", files[i]);
		}
	}
	if (Err)
		exit(1);
/*
 * Miscellaneous initialization.
 */

	for (i = 0; ; i++) {
		if (Pat[i].re == NULL)
			break;
		if ((Pat[i].pat = regcomp(Pat[i].re, 0)) == NULL)
			Error(WARN, NOLINE, Pat[i].re, " regcomp failure");
	}
	if ((i = Findscale('n', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['n']", NULL);
	Scalen = Scale[i].val;
	if ((i = Findscale('u', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['u']", NULL);
	Scaleu = Scale[i].val;
	if ((i = Findscale('v', 0.0, 0)) < 0)
		Error(WARN, NOLINE, " can't find Scale['v']", NULL);
	Scalev = Scale[i].val;
	(void) Findstr("CH", "= % -", 1);
	Cont = Newstr(" ");
	Contlen = 1;
	if (Err)
		exit(1);
/*
 * Here begins pass1 of awf - reading input lines and expanding macros.
 */

/*
 * Output prolog.
 */
	Macro(".^x");
	Macro(".^b");
	Macro(".^# 1 <prolog>");
/*
 * Read input files.
 */
	for (i = 0; i < nf; i++) {
		if (files[i] == NULL) {
			np = "stdin";
			Ifs = stdin;
		} else {
#ifdef	UNIX
			if ((Ifs = fopen(files[i], "r")) == NULL)
#else
			if ((Ifs = fopen(files[i], "rt")) == NULL)
#endif
				Error(FATAL, NOLINE, " can't open ", files[i]);
			np = files[i];
		}
		if (i >= pc) {
			(void) sprintf(Line, ".^# 1 %s", np);
			Macro(Line);
			NR = 0;
		}
		Fsp = 0;
		do {
			while (fgets(Line, MAXLINE, Ifs) != NULL) {
				NR++;
				if ((np = strrchr(Line, '\n')) != NULL)
					*np = '\0';
				else
					Line[MAXLINE-1] = '\0';
				Macro(Line);
			}
			if (i >= pc)
				Macro(".^e");
			if (Ifs != stdin)
				(void) fclose(Ifs);
			if (Fsp > 0) {
				Free(&Inname);
				Inname = Inn_stk[Fsp-1];
				NR = NR_stk[Fsp-1];
				Ifs = Ifs_stk[Fsp-1];
			}
		} while (Fsp-- > 0);
	}
	Macro(NULL);
	exit(Err);
}


/*
 * Macro(inp) - process a possible macro statement
 *		pass non-macros and macros alike to pass 2
 */

void
Macro(inp)
	char *inp;               /* possible macro statement pointer */
{
	char c[2];			/* characters */
	FILE *fs;			/* temporary file stream */
	int i, j, k;                    /* temporary indexes */
	int mx;                         /* Macrotab[] index */
	char *s1, *s2;                  /* temporary string pointers */

	if (inp == NULL) {
		Pass2(NULL);
		return;
	}
/*
 * Check for file name designator.
 */
	if (strncmp(inp, ".^#", 3) == 0) {
		Free(&Inname);
		Inname = Field(3, inp, 1);
		F = NULL;
		Pass2(inp);
		return;
	}
/*
 * Check for source command - "^\.so".
 */
	if (strncmp(inp, ".so", 3) == 0) {
		if ((s1 = Field(2, inp, 1)) == NULL) {
			Error(WARN, LINE, " no file specified", NULL);
			return;
		}
		if ((fs = fopen(s1, "r")) == NULL) {
			Error(WARN, LINE, " can't open", NULL);
			return;
		}
		if (Fsp >= MAXFSTK) {
			(void) fclose(fs);
			Error(WARN, LINE, " nesting too deep", NULL);
			return;
		}
		Ifs_stk[Fsp] = Ifs;
		Ifs = fs;
		Inn_stk[Fsp] = Inname;
		Inname = F;
		F = NULL;
		NR_stk[Fsp++] = NR;
		NR = 0;
		return;
	}
 /*
  * Check for start of macro definition.
  */
	if (strncmp(inp, ".de", 3) == 0) {
		if (inp[3] != ' ' || inp[4] == '\0') {
			Error(WARN, LINE, " illegal macro definition", NULL);
			return;
		}
		c[0] = inp[4];
		c[1] = inp[5];
		Curmx = Findmacro(c, 1);
		return;
	}
/*
 * Check for macro text.  Remove double backslashes.
 */
	if (Curmx >= 0 && strcmp(inp, "..") != 0) {
		if (Mtx >= MAXMTXT)
			Error(FATAL, LINE, " out of macro text space", NULL);
		if ((s1 = strchr(inp, '\\')) == NULL)
			Macrotxt[Mtx] = Newstr(inp);
		else {
			for (s1 = Pass1ln, s2 = inp;; s1++) {
				if ((*s1 = *s2++) == '\0')
					break;
				if (*s1 == '\\' && *s2 == '\\')
					s2++;
			}
			Macrotxt[Mtx] = Newstr(Pass1ln);
		}
		if (Macrotab[Curmx].bx == -1)
			Macrotab[Curmx].bx = Mtx;
		Mtx++;
		Macrotab[Curmx].ct++;
		return;
	}
/*
 * Check for end of macro.
 */
	if (Curmx >= 0 && strcmp(inp, "..") == 0) {
		Curmx = -1;
		(void) sprintf(Pass1ln, ".^# %d %s", NR, Inname);
		Pass2(Pass1ln);
		return;
	}
 /*
  * Check for conditionals and macro expansions.
  */
	if (*inp == '.'
	&&  (((mx = Findmacro(inp+1, 0)) != -1) || regexec(Pat[0].pat, inp))) {
		Expand(inp);
		return;
	}
/*
 * None of the above: forward the line.
 */
	Pass2(inp);
}
