/*
 *	pass2.c - cawf(1) pass 2 function
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
#else
#include <string.h>
#endif
#include <ctype.h>

/*
 * Pass2(line) - process the nroff commands in a line and break
 *		 text into words for pass 3
 */

void
Pass2(line)
	char *line;
{
	char buf[MAXLINE];		/* working buffer */
	char c;				/* character buffer */
	double d;			/* temporary double */
	double exscale;			/* expression scaling factor */
	double expr[MAXEXP];            /* expressions */
	char exsign[MAXEXP];            /* expression signs */
	int i, j, k;			/* temporary indexes */
	int inword;			/* word processing status */
	int nexpr;			/* number of expressions */
	char nm[4];			/* name */
	int nsp;			/* number of spaces */
	char op;			/* expression term operator */
	char opstack[MAXSP];		/* expression operation stack */
	char period;			/* end of word status */
	char *s1, *s2, *s3, *s4, *s5;	/* temporary string pointers */
	double sexpr[MAXEXP];           /* signed expressions */
	int sp;				/* expression stack pointer */
	char ssign;			/* expression's starting sign */
	int tabpos;			/* tab position */
	double tscale;			/* term scaling factor */
	double tval;			/* term value */
	double val;			/* term value */
	double valstack[MAXSP];		/* expression value stack */
	char xbuf[MAXLINE];		/* expansion buffer */

	if (line == NULL) {
    /*
     * End of macro expansion.
     */
		Pass3(DOBREAK, "need", NULL, 999);	/* flush page */
		return;
	}
    /*
     * Adjust line number.
     */
	if (Lockil == 0)
		P2il++;
    /*
     * Empty line - "^[ \t]*$".
     */
	if (regexec(Pat[6].pat, line)) {
		Pass3(DOBREAK, "space", NULL, 0);
		return;
	}
    /*
     * Line begins with white space.
     */
	if (*line == ' ' || *line == '\t') {
		Pass3(DOBREAK, "flush", NULL, 0);
		Pass3(0, "", NULL, 0);
	}
    /*
     * Line contains text (not an nroff command).
     */
	if (*line != '.') {
		if (Font[0] == 'R' && Backc == 0 && Aftnxt == NULL
		&&  regexec(Pat[7].pat, line) == 0) {
		    /*
		     * The font is Roman, there is no "\\c" or "after next"
		     * trap pending and and the line has no '\\', '\t', '-',
		     * or "  "  (regular expression "\\|\t|-|  ").
		     *
		     * Output each word of the line as "<length> <word>".
		     */
			for (s1 = line;;) {
				while (*s1 && *s1 == ' ')
					s1++;
				if (*s1 == '\0')
					break;
				for (s2 = s1, s3 = buf; *s2 && *s2 != ' ';)
					*s3++ = *s2++;
				*s3 = '\0';
				Pass3((s2 - s1), buf, NULL, 0);
				s1 = *s2 ? ++s2 : s2;
			}
		    /*
		     * Line terminates with punctuation and optional
		     * bracketing (regular expression "[.!?:][\])'\"*]*$").
		     */
			if (regexec(Pat[8].pat, line))
				Pass3(NOBREAK, "gap", NULL, 2);
			if (Centering > 0) {
				Pass3(DOBREAK,"center", NULL, 0);
				Centering--;
			} else if (Fill == 0)
				Pass3(DOBREAK, "flush", NULL, 0);
			return;
		}
	    /*
	     * Line must be scanned a character at a time.
	     */
		inword = nsp = tabpos = 0;
		period = '\0';
		for (s1 = line;; s1++) {
		    /*
		     * Space or TAB causes state transition.
		     */
			if (*s1 == '\0' || *s1 == ' ' || *s1 == '\t') {
				if (inword) {
					if (!Backc) {
						Word[Wordx] = '\0';
						Pass3(Wordl, Word, NULL, 0);
						if (Uhyph) {
						    Pass3(NOBREAK, "nohyphen",
							NULL, 0);
						}
					}
					inword = 0;
					nsp = 0;
				}
				if (*s1 == '\0')
					break;
			} else {
				if (inword == 0) {
					if (Backc == 0) {
						Wordl = Wordx = 0;
						Uhyph = 0;
					}
					Backc = 0;
					inword = 1;
					if (nsp > 1) {
						Pass3(NOBREAK, "gap", NULL,
						    nsp);
					}
				}
			}
		    /*
		     * Process a character.
		     */
			switch (*s1) {
		    /*
		     * Space
		     */
	     		case ' ':
				nsp++;
				period = '\0';
				break;
		    /*
		     * TAB
		     */
	     		case '\t':
				tabpos++;
				if (tabpos <= Ntabs) {
					Pass3(NOBREAK, "tabto", NULL,
					    Tabs[tabpos-1]);
				}
				nsp = 0;
				period = '\0';
				break;
		    /*
		     * Hyphen if word is being assembled
		     */
			case '-':
				if (Wordl <= 0)
				    goto ordinary_char;
				if ((i = Findhy(NULL, 0, 0)) < 0) {
				    Error(WARN, LINE, " no hyphen for font ",
					Font);
				    return;
				}
				Word[Wordx] = '\0';
				Pass3(Wordl, Word, NULL, Hychar[i].len);
				Pass3(NOBREAK, "userhyphen", Hychar[i].str,
				    Hychar[i].len);
				Wordl = Wordx = 0;
				period = '\0';
				Uhyph = 1;
				break;
		    /*
		     * Backslash
		     */
			case '\\':
				s1++;
				switch(*s1) {
			    /*
			     * Change font - "\\fN"
			     */
				case 'f':
					s1 = Asmcode(&s1, nm);
					if (nm[0] == 'P') {
					    Font[0] = Prevfont;
					    break;
					}
					for (i = 0; Fcode[i].nm; i++) {
					    if (Fcode[i].nm == nm[0])
						break;
					}
					if (Fcode[i].nm == '\0'
					||  nm[1] != '\0') {
					    Error(WARN, LINE, " unknown font ",
					    	nm);
					    break;
					}
					if (Fcode[i].status != '1') {
					    Error(WARN, LINE,
						" font undefined ", nm);
					    break;
					} else {
					    Prevfont = Font[0];
					    Font[0] = nm[0];
					}
					break;
			    /*
			     * Positive horizontal motion - "\\h\\n(NN" or
			     * "\\h\\nN"
			     */
				case 'h':
					if (s1[1] != '\\' || s1[2] != 'n') {
					    Error(WARN, LINE,
						" no \\n after \\h", NULL);
					    break;
					}
					s1 +=2;
					s1 = Asmcode(&s1, nm);
					if ((i = Findnum(nm, 0, 0)) < 0)
						goto unknown_num;
					if ((j = Numb[i].val) < 0) {
					    Error(WARN, LINE, " \\h < 0 ",
					    NULL);
					    break;
					}
					if (j == 0)
						break;
					if ((strlen(s1+1) + j + 1) >=  MAXLINE)
						goto line_too_long;
					for (s2 = &xbuf[1]; j; j--)
						*s2++ = ' ';
					(void) strcpy(s2, s1+1);
					s1 = xbuf;
					break;
			    /*
			     * Interpolate number - "\\n(NN" or "\\nN"
			     */
				case 'n':
					s1 = Asmcode(&s1, nm);
					if ((i = Findnum(nm, 0, 0)) < 0) {
unknown_num:
					    Error(WARN, LINE,
					        " unknown number register ",
						nm);
					    break;
					}
					(void) sprintf(buf, "%d",
					    Numb[i].val);
					if ((strlen(buf) + strlen(s1+1) + 1)
					>=  MAXLINE) {
line_too_long:
					    Error(WARN, LINE, " line too long",
					        NULL);
					    break;
					}
					(void) sprintf(buf, "%d%s",
					    Numb[i].val, s1+1);
					(void) strcpy(&xbuf[1], buf);
				        s1 = xbuf;
					break;
			    /*
			     * Change size - "\\s[+-][0-9]" - NOP
			     */
				case 's':
					s1++;
					if (*s1 == '+' || *s1 == '-')
						s1++;
					while (*s1 && isdigit(*s1))
						s1++;
					s1--;
					break;
			    /*
			     * Continue - "\\c"
			     */
				case 'c':
					Backc = 1;
					break;
			    /*
			     * Interpolate string - "\\*(NN" or "\\*N"
			     */
				case '*':
					s1 = Asmcode(&s1, nm);
					s2 = Findstr(nm, NULL, 0);
					if (*s2 != '\0') {
					    if ((strlen(s2) + strlen(s1+1) + 1)
					    >=  MAXLINE)
						goto line_too_long;
					    (void) sprintf(buf, "%s%s",
						s2, s1+1);
					    (void) strcpy(&xbuf[1], buf);
					    s1 = xbuf;
					}
					break;
			    /*
			     * Discretionary hyphen - "\\%"
			     */
				case '%':
					if (Wordl <= 0)
					    break;
					if ((i = Findhy(NULL, 0, 0)) < 0) {
					    Error(WARN, LINE,
					        " np hyphen for font ", Font);
					    break;
					}
					Word[Wordx] = '\0';
					Pass3(Wordl, Word, NULL, Hychar[i].len);
					Pass3(NOBREAK, "hyphen", Hychar[i].str,
					    Hychar[i].len);
					Wordl = Wordx = 0;
					Uhyph = 1;
					break;
			    /*
			     * None of the above - may be special character
			     * name.
			     */
				default:
					s2 = --s1;
					s1 = Asmcode(&s1, nm);
					if ((i = Findchar(nm, 0, 0, 0)) < 0) {
					    s1 = s2;
					    goto ordinary_char;
					}
					if (strcmp(nm, "em") == 0
					&& Wordx > 0) {
				    /*
				     * "\\(em" is a special case when a word
				     * has been assembled, because of
				     * hyphenation.
				     */
					    Word[Wordx] = '\0';
					    Pass3(Wordl, Word, NULL,
					        Schar[i].len);
					    Pass3(NOBREAK, "userhyphen",
					        Schar[i].str, Schar[i].len);
				            Wordl = Wordx = 0;
					    period = '\0';
					    Uhyph = 1;
			 		}
				    /*
				     * Interpolate a special character
				     */
					if ((strlen(Schar[i].str) + Wordx)
					>= MAXLINE)
					    goto word_too_long;
				        for (s2 = Schar[i].str; *s2; s2++)
					    Word[Wordx++] = *s2;
				        Wordl += Schar[i].len;
					period = '\0';
				}
				break;
		    /*
		     * Ordinary character
		     */
			default:
ordinary_char:
				if ((Font[0] == 'B' && (Wordx+5) >= MAXLINE)
				||  (Font[0] == 'I' && (Wordx+3) >= MAXLINE)) {
word_too_long:
					Error(WARN, LINE, " word too long",
					    NULL);
					return;
				}
				if (Font[0] == 'B') {
				    Word[Wordx++] = *s1;
				    Word[Wordx++] = '\b';
				    Word[Wordx++] = *s1;
				    Word[Wordx++] = '\b';
				    Word[Wordx++] = *s1;
				} else if (Font[0] == 'I' && isalnum(*s1)) {
				    Word[Wordx++] = '_';
				    Word[Wordx++] = '\b';
				    Word[Wordx++] = *s1;
				} else {
				    if ((Wordx+1) >= MAXLINE)
					goto word_too_long;
				    Word[Wordx++] = *s1;
				}
				Wordl++;
				if (*s1 == '.' || *s1 == '!'
				||  *s1 == '?' || *s1 == ':')
				    period = '.';
				else if (period == '.') {
				    nm[0] = *s1;
				    nm[1] = '\0';
				    if (regexec(Pat[13].pat, nm) == 0)
					 period = '\0';
				}
			}
		}
	    /*
	     * End of line processing
	     */
     		if (!Backc) {
			if (period == '.')
				Pass3(NOBREAK, "gap", NULL, 2);
			if (Centering > 0) {
				Pass3(DOBREAK, "center", NULL, 0);
				Centering--;
			} else if (!Fill)
				Pass3(DOBREAK, "flush", NULL, 0);
		}
		if (Aftnxt == NULL)
			return;
		/* else fall through to process an "after next trap */
	}
    /*
     * Special -man macro handling.
     */
	if (Marg == MANMACROS) {
	    /*
	     * A text line - "^[^.]" - is only processed when there is an
	     * "after next" directive.
	     */
		if (*line != '.') {
			if (Aftnxt != NULL) {
				if (regexec(Pat[9].pat, Aftnxt))  /* ",fP" */
					Font[0] = Prevfont;
				if (regexec(Pat[10].pat, Aftnxt))  /* ",tP" */
					Pass3(DOBREAK, "toindent", NULL, 0);
				Free(&Aftnxt);
			}
			return;
		}
	    /*
	     * Special footer handling - "^.lF"
	     */
		if (line[1] == 'l' && line[2] == 'F') {
			s1 = Findstr("by", NULL, 0);
			s2 = Findstr("nb", NULL, 0);
			if (*s1 == '\0' || *s2 == '\0')
				(void) sprintf(buf, "%s%s", s1, s2);
			else
				(void) sprintf(buf, "%s; %s", s1, s2);
			Pass3(NOBREAK, "LF", buf, 0);
			return;
		}
	}
    /*
     * Special -ms macro handling.
     */
	if (Marg == MSMACROS) {
	    /*
	     * A text line - "^[^.]" - is only processed when there is an
	     * "after next" directive.
	     */
		if (*line != '.') {
			if (Aftnxt != NULL) {
				if (regexec(Pat[10].pat, Aftnxt))  /* ",tP" */
					Pass3(DOBREAK, "toindent", NULL, 0);
				Free(&Aftnxt);
			}
			return;
		}
	    /*
	     * Numbered headings - "^\.nH"
	     */
		if (line[1] == 'n' && line[2] == 'H') {
			s1 = Field(2, line, 0);
			if (s1 != NULL) {
				i = atoi(s1) - 1;	
				if (i < 0) {
					for (j = 0; j < MAXNHNR; j++) {
						Nhnr[j] = 0;
					}
					i = 0;
				} else if (i >= MAXNHNR) {
				    (void) sprintf(buf, " over NH limit (%d)",
					MAXNHNR);
				    Error(WARN, LINE, buf, NULL);
				}
			} else
				i = 0;
			Nhnr[i]++;
			for (j = i + 1; j < MAXNHNR; j++) {
				Nhnr[j] = 0;
			}
			s1 = buf;
			for (j = 0; j <= i; j++) {
				(void) sprintf(s1, "%d.", Nhnr[j]);
				s1 += strlen(buf);
			}
			(void) Findstr("Nh", buf, 1);
			return;
		}
	}
    /*
     * Remaining lines should begin with a '.' unless an "after next"
     * trap has failed.
     */
	if (line[0] != '.') {
		if (Aftnxt != NULL)
			Error(WARN, LINE, " failed .it: ", Aftnxt);
		else
			Error(WARN, LINE, " unrecognized line ", NULL);
		return;
	}
    /*
     * Evaluate expressions for "^\.(ta|ll|ls|in|ti|po|ne|sp|pl|nr)"
     * Then process the commands.
     */
	if (regexec(Pat[11].pat, &line[1])) {
	    /*
	     * Establish default scale factor.
	     */
		if ((line[1] == 'n' && line[2] == 'e')
		||  (line[1] == 's' && line[2] == 'p')
		||  (line[1] == 'p' && line[2] == 'l'))
			exscale = Scalev;
		else if (line[1] == 'n' && line[2] == 'r')
			exscale = Scaleu;
		else
			exscale = Scalen;
	    /*
	     * Determine starting argument.
	     */
		if (line[1] == 'n' && line[2] == 'r')
			s1 = Field(3, line, 0);
		else
			s1 = Field(2, line, 0);
	    /*
	     * Evaluate expressions.
	     */
		for (nexpr = 0; s1 != NULL &&*s1 != '\0'; ) {
			while (*s1 == ' ' || *s1 == '\t')
				s1++;
			if (*s1 == '+' || *s1 == '-')
				ssign = *s1++;
			else
				ssign = '\0';
		    /*
		     * Process terms.
		     */
			val = 0.0;
			sp = -1;
			c = '+';
			s1--;
			while (c == '+' || c == '*' || c == '%'
			||  c == ')' || c == '-' || c == '/') {
			    op = c;
			    s1++;
			    tscale = exscale;
			    tval = 0.0;
			/*
			 * Pop stack on right parenthesis.
			 */
			    if (op == ')') {
				tval = val;
				if (sp >= 0) {
				    val = valstack[sp];
				    op = opstack[sp];
				    sp--;
				} else {
				    Error(WARN, LINE,
					" expression stack underflow", NULL);
				    return;
				}
				tscale = Scaleu;
			/*
			 * Push stack on left parenthesis.
			 */
			    } else if (*s1 == '(') {
				sp++;
				if (sp >= MAXSP) {
				    Error(WARN, LINE,
				       " expression stack overflow", NULL);
				    return;
				}
				valstack[sp] = val;
				opstack[sp] = op;
				val = 0.0;
				c = '+';
				continue;
			    } else if (*s1 == '\\') {
			      s1++;
			      switch(*s1) {
			/*
			 * "\\"" begins a comment.
			 */
			      case '"':
				while (*s1)
					s1++;
				break;;
			/*
			 * Crude width calculation for "\\w"
			 */
			      case 'w':
				s2 = ++s1;
				if (*s1) {
				    s1++;
				    while (*s1 && *s1 != *s2)
					s1++;
				    tval = (double) (s1 - s2 - 1) * Scalen;
				    if (*s1)
					s1++;
				}
				break;
			/*
			 * Interpolate number register if "\\n".
			 */
			      case 'n':
				i = (s1[1] == '(') ? 2: 1;
				s1 += i;
			        if (*s1 && *s1 != ' ' && *s1 != '\t')
					nm[0] = *s1++;
				else
					nm[0] = '\0';
				if (i == 2 && *s1 && *s1 != ' ' && *s1 != '\t')
					nm[1] = *s1++;
				else
					nm[1] = '\0';
				if ((i = Findnum(nm, 0, 0)) >= 0)
					tval = Numb[i].val;
			     }
			/*
			 * Assemble numeric value.
			 */
			    } else if (*s1 == '.' || isdigit(*s1)) {
				for (i = 0; isdigit(*s1) || *s1 == '.'; s1++) {
				    if (*s1 == '.') {
					i = 10;
					continue;
				    }
				    d = (double) (*s1 - '0');
				    if (i) {
					tval = tval + (d / (double) i);
					i = i * 10;
				    } else
					tval = (tval * 10.0) + d;
				}
			    } else {
			/*
			 * It's not an expression.  Ignore extra scale.
			 */
				if ((i = Findscale(*s1, 0.0, 0)) < 0) {
				    (void) sprintf(buf,
					" \"%s\" isn't an expression", s1);
				    Error(WARN, LINE, buf, NULL);
				}
				s1++;
			    }
			/*
			 * Add term to expression value.
			 */
			    if ((i = Findscale(*s1, 0.0, 0)) >= 0) {
				tval *= Scale[i].val;
				s1++;
			    } else
				tval *= tscale;
			    switch (op) {
			    case '+':
				val += tval;
				break;
			    case '-':
				val -= tval;
				break;
			    case '*':
				val *= tval;
				break;
			    case '/':
			    case '%':
				i = (int) val;
				j = (int) tval;
				if (j == 0) {
				    Error(WARN, LINE,
					(*s1 == '/') ? "div" : "mod",
				        " by 0");
				    return;
				}
				if (op == '/')
					val = (double) (i / j);
				else
					val = (double) (i % j);
				break;
			    }
			    c = *s1;
			}
		    /*
		     * Save expression value and sign.
		     */
			if (nexpr >= MAXEXP) {
				(void) sprintf(buf,
				    " at expression limit of %d", MAXEXP);
				Error(WARN, LINE, buf, NULL);
				return;
			}
			exsign[nexpr] = ssign;
			expr[nexpr] = val;
			if (ssign == '-')
				sexpr[nexpr] = -1.0 * val;
			else
				sexpr[nexpr] = val;
			nexpr++;
			while (*s1 == ' ' || *s1 == '\t')
				s1++;
		}
	    /*
	     * Set parameters "(ll|ls|in|ti|po|pl)"
	     */
		if (regexec(Pat[12].pat, &line[1])) {
			nm[0] = line[1];
			nm[1] = line[2];
			if ((i = Findparms(nm)) < 0) {
				Error(WARN, LINE,
				    " can't find parameter register ", nm);
				return;
			}
			if (nexpr == 0 || exscale == 0.0)
				j = Parms[i].prev;
			else if (exsign[0] == '\0'
			     ||  (nm[0] == 't' && nm[1] == 'i'))
				 j = (int)(expr[0] / exscale);
			else
				j = Parms[i].val + (int)(sexpr[0] / exscale);
			Parms[i].prev = Parms[i].val;
			Parms[i].val = j;
			Pass3(DOBREAK, Parms[i].cmd, NULL, j);
			return;
		}
		if (line[1] == 'n') {
			switch(line[2]) {
	    /*
	     * Need - "^\.ne <expression>"
	     */
			case 'e':
				if (nexpr && Scalev > 0.0)
					i = (int) ((expr[0]/Scalev) + 0.99);
				else
					i = 0;
				Pass3(DOBREAK, "need", NULL, i);
				return;
	    /*
	     * Number - "^\.nr <name> <expression>"
	     */
			case 'r':
				if ((s1 = Field(2, line, 0)) == NULL) {
				    Error(WARN, LINE, " bad number register",
				        NULL);
				    return;
				}
				if ((i = Findnum(s1, 0, 0)) < 0)
				    i = Findnum(s1, 0, 1);
				if (nexpr < 1) {
				    Numb[i].val = 0;
				    return;
				}
				if (exsign[0] == '\0')
				    Numb[i].val = (int) expr[0];
				else
				    Numb[i].val += (int) sexpr[0];
				return;
			}
		}
	    /*
	     * Space - "^\.sp <expression>"
	     */
		if (line[1] == 's' && line[2] == 'p') {
			if (nexpr == 0)
				i = 1;
			else
				i = (int)((expr[0] / Scalev) + 0.99);
			while (i--)
				Pass3(DOBREAK, "space", NULL, 0);
			return;
		}
	    /*
	     * Tab positions - "^\.ta <pos1> <pos2> . . ."
	     */
     		if (line[1] == 't' && line[2] == 'a') {
			tval = 0.0;
			for (j = 0; j < nexpr; j++) {
				if (exsign[j] == '\0')
					tval = expr[j];
				else
					tval += sexpr[j];
				Tabs[j] = (int) (tval / Scalen);
			}
			Ntabs = nexpr;
			return;
		}
	}
    /*
     * Remaining lines begin with a '.'.
     */

    /*
     * Adjust - "^\.ad"
     */
	if (line[1] == 'a' && line[2] == 'd') {
		Pass3(NOBREAK, "both", NULL, 0);
		return;
	}
	if (line[1] == 'b') {
		switch (line[2]) {
    /*
     * Break - "^\.br"
     */
		case 'r':
			Pass3(DOBREAK, "flush", NULL, 0);
			return;
    /*
     * Begin new page - "^\.bp"
     */
		case 'p':
			Pass3(DOBREAK, "need", NULL, 999);
			return;
		}
	}
    /*
     * Center - "^\.ce"
     */
	if (line[1] == 'c' && line[2] == 'e') {
		if ((s2 = Field(2, line, 0)) != NULL)
			Centering = atoi(s2);
		else
			Centering = 1;
		return;
	}
    /*
     * Define string - "^\.ds"
     */
	if (line[1] == 'd' && line[2] == 's') {

common_ds:

		if (Asmname(&line[3], nm) == 0) {
no_name:
			Error(WARN, LINE, " no name", NULL);
			return;
		}
		s3 = Field(3, line, 0);
		s4 = Findstr(nm, s3, 1);
		if (Hdft) {
		    /*
		     * Look for names LH, LF, CH, CF, RH, RF.
		     */
			if ((nm[0] == 'L' || nm[0] == 'C' || nm[0] == 'R')
			&&  (nm[1] == 'F' || nm[1] == 'H')) {
				(void) sprintf(buf, "%s", nm);
				Pass3(NOBREAK, buf, s4, 0);
			}
		}
		return;
	}
	if (line[1] == 'f') {
    /*
     * Fill - "^\.fi"
     */
		if (line[2] == 'i') {
			Fill = 1;
			Pass3(DOBREAK, "flush", NULL, 0);
			return;
		}
    /*
     * Font - "^\.ft <font_name>"
     */
		if (line[2] == 't') {
			if (line[3] == '\0' || line[4] == '\0')
				line[4] = 'P';
			if (line[4] == 'P') {
				Font[0] = Prevfont;
				return;
			}
			for (i = 0; Fcode[i].nm; i++) {
				if (Fcode[i].nm == line[4])
					break;
			}
			if (Fcode[i].status == '\0') {
				Error(WARN, LINE, " bad font code", NULL);
				return;
			}
			Prevfont = Font[0];
			Font[0] = line[4];
			return;
		}
	}
    /*
     * Input trap - "^\.it [1 <command>]"
     */
	if (line[1] == 'i' && line[2] == 't') {
		if ((s2 = Field(2, line, 0)) == NULL) {
			Free(&Aftnxt);
			return;
		}
		if ((i = atoi(s2)) != 1) {
			Error(WARN, LINE, " first .it arg must be 1", NULL);
			return;
		}
		if ((s3 = Field(3, line, 0)) == NULL)
			Free(&Aftnxt);
		else {
			(void) sprintf(buf, "%s,%s",
				(Aftnxt == NULL) ? "" : Aftnxt, s3);
			Free(&Aftnxt);
			Aftnxt = Newstr(buf);
		}
		return;
	}
    /*
     * "^\.i0", "^\.lg" and "^\.li" - do nothing
     */
	if ((line[1] == 'i' && line[2] == '0')
	||  (line[1] == 'l' && line[2] == 'g')
	||  (line[1] == 'l' && line[2] == 'i'))
		return;
	if (line[1] == 'n') {
		switch (line[2]) {
    /*
     * No adjust "^\.na"
     */
		case 'a':
			Pass3(NOBREAK, "left", NULL, 0);
			return;
    /*
     * No fill - "^\.nf"
     */
		case 'f':
			Fill = 0;
			Pass3(DOBREAK, "flush", NULL, 0);
			return;
    /*
     * No space - "^\.ns"
     */
		case 's':
			Pass3(NOBREAK, "nospace", NULL, 0);
			return;
		}
	}
    /*
     * Point size - "^\.ps"
     */
	if (line[1] == 'p' && line[2] == 's')
		return;
	if (line[1] == 'r') {
		switch (line[2]) {
    /*
     * Remove macro or string - "^\.rm"
     */
		case 'm':
			if (Asmname(&line[3], nm) == 0)
				goto no_name;
			if ((i = Findmacro(nm, 0)) >= 0) {
				Delmacro(i);
				return;
			}
			(void) Findstr(nm, NULL, 0);
			if (Sx < 0) {
				Error(WARN, LINE, " no macro/string", NULL);
				return;
			}
			Delstr(Sx);
			return;
    /*
     * Remove register - "^\.rr"
     */
		case 'r':
			if (Asmname(&line[3], nm) == 0)
				goto no_name;
			if ((i = Findnum(nm, 0, 0)) < 0) {
				Error(WARN, LINE, " no register", NULL);
				return;
			}
			Delnum(i);
			return;
    /*
     * Resume space - "^\.rs"
     */
		case 's':
			Pass3(NOBREAK, "yesspace", NULL, 0);
			return;
		}
	}
    /*
     * Message - "^\.tm"
     */
	if (line[1] == 't' && line[2] == 'm') {
		Pass3(MESSAGE, Inname,
			(line[3] == ' ') ? &line[4] : &line[3], NR);
		return;
	}
    /*
     * Vertical spacing - "^\.vs" (do nothing)
     */
	if (line[1] == 'v' && line[2] == 's')
		return;
	if (line[1] == '^') {
		switch(line[2]) {
    /*
     * Initialization - "^\.\^b (fh|HF|NH) [01]"
     *
     * fh = first page header status
     * HF = header/footer status
     * NH = initialize number headers
     */
		case 'b':
			if ((s1 = Field(2, line, 0)) == NULL)
				return;
			if ((s2 = Field(3, line, 0)) == NULL)
				i = 0;
			else
				i = atoi(s2);
			if (s1[0] == 'f' && s1[1] == 'h')
				Pass3(NOBREAK, "fph", NULL, i);
			else if (s1[0] == 'H' && s1[1] == 'F')
				Hdft = i;
			else if (s1[0] == 'N' && s1[1] == 'H') {
				for (i = 0; i < MAXNHNR; i++)
					Nhnr[i] = 0;
			} else
				Error(WARN, LINE, " unknown initialization",
					NULL);
			return;
    /*
     * Character definitions - "^\.\^c"
     */
		case 'c':

			s2 = Field(2, line, 0);
			i = atoi(Field(3, line, 0));
			s4 = Field(4, line, 0);
			if (i < 0 || i > MAXLINE/2 || *s2 == '\0') {
				Error(WARN, LINE, " bad character definition",
					NULL);
				return;
			}
			if (s4 == NULL)
				s4 = "";
			else if (*s4 == '"')
				s4++;
			s1 = buf;
			while ((s5 = strchr(s4, '\\')) != NULL) {
				while (s5 > s4)
					*s1++ = *s4++;
				s4 = ++s5;
				if (*s5 == '\\')
					*s1++ = '\\';
				else if (*s5 == 'b')
					*s1++ = '\b';
				if (*s4)
					s4++;
			}
			while (*s1++ = *s4++)
				;
			if (*s2 == 'h' && *(s2+1) == 'y')
				(void) Findhy(buf, i, 1);
			else
				(void) Findchar(s2, i, buf, 1);
			return;
    /*
     * Debug - "^\.\^d"
     */
		case 'd':
			return;
    /*
     * Finalization - "\.\^e"
     */
		case 'e':
			return;
    /*
     * Font is OK - "\.\^f <font_name_character>"
     */
		case 'f':
			if (line[3] != '\0' && line[4] != '\0') {
				for (i = 0; Fcode[i].nm; i++) {
					if (line[4] == Fcode[i].nm) {
						Fcode[i].status = '1';
						return;
					}
				}
			}
			Error(WARN, LINE, " unknown font", NULL);
			return;
    /*
     * Resolutions - "\.\^r cpi horizontal vertical"
     */
		case 'r':
			if ((i = atoi(Field(3, line, 0))) <= 0
			||  (j = atoi(Field(4, line, 0))) <= 0) {
				Error(WARN, LINE, " bad cpi resolutions",
					NULL);
				return;
			}
			tval = (double) (240.0 / (double) i);
			if (Findscale('m', tval, 1) < 0)
				Error(FATAL, LINE, " missing Scal['m']",
					NULL);
			Scalen = tval;
			if (Scalen <= 0.0) {
				(void) sprintf(buf, " bad Scale['n'] (%f)",
					Scalen);
				Error(FATAL, LINE, buf, NULL);
			}
			if (Findscale('n', tval, 1) < 0)
				Error(FATAL, LINE, " missing Scale['n']",
					NULL);
			Scalev = (double) (240.0 / (double) j);
			if (Scalev <= 0.0) {
				(void) sprintf(buf, " bad Scale['v'] (%f)",
					Scalen);
				Error(FATAL, LINE, buf, NULL);
			}
			if (Findscale('v', Scalev, 1) < 0)
				Error(FATAL, LINE, " missing Scale['v']",
					NULL);
			return;
    /*
     * Error file - "^\.\^x <name>"
     */
		case 'x':
			return;
    /*
     * Set line number and file name - "^\.\^# <number> <file>"
     *
     * Lock line number and file name - "^\.\^= <number> <file>"
     */
		case '#':
		case '=':
			if ((s1 = Field(2, line, 0)) != NULL)
				P2il = atoi(s1) - 1;
			else
				P2il = 0;
			Lockil = (line[2] == '#') ? 0 : 1;
			Free(&P2name);
			if (Field(3, line, 1) != NULL) {
				P2name = F;
				F = NULL;
			} else
				P2name = NULL;
			return;
		}
	}
    /*
     * Comment - "^\.\\"
     */
	if (line[1] == '\\')
		return;
    /*
     * Unknown command starting with a '.'.
     */
	Error(WARN, LINE, " unknown command", NULL);
	return;
}
