/*
 *	This library functions were written by id Software.
 *
 *	Copyright (C) 1994 by id Software
 *
 *	I have modified this some to get it integrated into the tool
 *	collection. Please don't bother anyone at id Software about
 *	this, if you have a problem you could try to email me.
 *
 *	24 June 1998, Udo Munk	munkudo@aol.com
 */

#include "cmdlib.h"
#include "scriplib.h"

/*
=============================================================================

						PARSING STUFF

=============================================================================
*/

char	token[MAXTOKEN];
char	*scriptbuffer,*script_p,*scriptend_p;
int	grabbed;
int	scriptline;
boolean endofscript;
boolean tokenready;	// only true if UnGetToken was just called

/*
==============
=
= LoadScriptFile
=
==============
*/

void LoadScriptFile (char *filename)
{
	long size;

	size = LoadFile (filename, (void **)&scriptbuffer);

	script_p = scriptbuffer;
	scriptend_p = script_p + size;
	scriptline = 1;
	endofscript = false;
	tokenready = false;
}

/*
==============
=
= UnGetToken
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

= could cross a line boundary.
=
==============
*/

void UnGetToken (void)
{
	tokenready = true;
}

/*
==============
=
= GetToken
=
==============
*/

void GetToken (boolean crossline)
{
	char *token_p;

	if (tokenready)                         // is a token allready waiting?
	{
		tokenready = false;
		return;
	}

	if (script_p >= scriptend_p)
	{
		if (!crossline)
			Error ("Line %i is incomplete\n", scriptline);
		endofscript = true;
		return;
	}

//
// skip space
//
skipspace:
	while (*script_p <= 32)
	{
		if (script_p >= scriptend_p)
		{
			if (!crossline)
				Error ("Line %i is incomplete\n", scriptline);
			endofscript = true;
			return;
		}
		if (*script_p++ == '\n')
		{
			if (!crossline)
				Error ("Line %i is incomplete\n", scriptline);
			scriptline++;
		}
	}

	if (script_p >= scriptend_p)
	{
		if (!crossline)
			Error ("Line %i is incomplete\n", scriptline);
		endofscript = true;
		return;
	}

	if (*script_p == ';' || *script_p == '#') // semicolon is comment field
	{											// also make # a comment field
		if (!crossline)
			Error ("Line %i is incomplete\n", scriptline);
		while (*script_p++ != '\n')
			if (script_p >= scriptend_p)
			{
				endofscript = true;
				return;
			}
		goto skipspace;
	}

//
// copy token
//
	token_p = token;

	while (*script_p > 32 && *script_p != ';')
	{
		*token_p++ = *script_p++;
		if (script_p == scriptend_p)
			break;
		if (token_p == &token[MAXTOKEN])
			Error ("Token too large on line %i\n", scriptline);
	}

	*token_p = 0;
}

/*
==============
=
= TokenAvailable
=
= Returns true if there is another token on the line
=
==============
*/

boolean TokenAvailable (void)
{
	char *search_p;

	search_p = script_p;

	if (search_p >= scriptend_p)
		return false;

	while (*search_p <= 32)
	{
		if (*search_p == '\n')
			return false;
		search_p++;
		if (search_p == scriptend_p)
			return false;
	}

	if (*search_p == ';')
		return false;

	return true;
}
