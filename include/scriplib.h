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

#ifndef __SCRIPLIB_H__
#define __SCRIPLIB_H__

#define	MAXTOKEN 128

extern	char	token[MAXTOKEN];
extern	char	*scriptbuffer,*script_p,*scriptend_p;
extern	int	grabbed;
extern	int	scriptline;
extern	boolean	endofscript;


void LoadScriptFile (char *filename);
void GetToken (boolean crossline);
void UnGetToken (void);
boolean TokenAvailable (void);

#endif
