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

#include <stdio.h>

#include "sysdep.h"
#include "strfunc.h"
#include "cmdlib.h"


/* globals for command line args */
#if defined(UNIX) || defined(MSDOS)
int NXArgc;
char **NXArgv;
#define myargc  NXArgc
#define myargv  NXArgv

#elif defined(RISCOS)
#include "unixlike.h"
int myargc;
char **myargv;

#else
#ifdef __WATCOMC__
extern int _argc;
extern char **_argv;
#define myargc  _argc
#define myargv  _argv
#endif

#endif

/*
=============================================================================

						MISC FUNCTIONS

=============================================================================
*/

/*
=================
=
= Error
=
= For abnormal program terminations
=
=================
*/

void Error (char *error, ...)
{
	va_list argptr;

#if !defined(UNIX) && !defined(RISCOS) && !defined(MSDOS)
	if ( *(byte *)0x449 == 0x13)
		TextMode ();
#endif

	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");
	exit (1);
}

/*
=================
=
= CheckParm
=
= Checks for the given parameter in the program's command line arguments
=
= Returns the argument number (1 to argc-1) or 0 if not present
=
=================
*/

int CheckParm (char *check)
{
	int	i;
	char	*parm;

	for (i = 1; i < myargc; i++)
	{
		parm = myargv[i];

		if ( !isalpha(*parm) )	  // skip - / \ etc.. in front of parm
			if (!*++parm)
				continue; // parm was only one char

		if ( !stricmp(check,parm) )
			return i;
	}

	return 0;
}

int SafeOpenWrite (char *filename)
{
	int handle;

	handle = open(filename, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, 0666);

	if (handle == -1)
		Error ("Error opening %s: %s", filename, strerror(errno));

	return handle;
}

int SafeOpenRead (char *filename)
{
	int handle;

	handle = open(filename, O_RDONLY | O_BINARY);

	if (handle == -1)
		Error ("Error opening %s: %s", filename, strerror(errno));

	return handle;
}

void SafeRead (int handle, void *buffer, long count)
{
	unsigned iocount;

	while (count)
	{
		iocount = (unsigned)(count > 0x8000 ? 0x8000 : count);
		if (read (handle, buffer, iocount) != iocount)
			Error ("File read failure");
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
}

void SafeWrite (int handle, void *buffer, long count)
{
	unsigned iocount;

	while (count)
	{
		iocount = (unsigned)(count > 0x8000 ? 0x8000 : count);
		if (write (handle, buffer, iocount) != iocount)
			Error ("File write failure");
		buffer = (void *)( (byte *)buffer + iocount );
		count -= iocount;
	}
}

void *SafeMalloc (long size)
{
	void *ptr;

	ptr = malloc ((int)size);

	if (!ptr)
		Error ("Malloc failure for %lu bytes", size);

	return ptr;
}

/*
==============
=
= LoadFile
=
==============
*/

long LoadFile (char *filename, void **bufferptr)
{
	int	handle;
	long	length;
	void	*buffer;

	handle = SafeOpenRead (filename);
	length = filelength (handle);
	buffer = SafeMalloc (length);
	SafeRead (handle, buffer, length);
	close (handle);

	*bufferptr = buffer;
	return length;
}

/*
==============
=
= SaveFile
=
==============
*/

void SaveFile (char *filename, void *buffer, long count)
{
	int handle;

	handle = SafeOpenWrite (filename);
	SafeWrite (handle, buffer, count);
	close (handle);
}

void DefaultExtension (char *path, char *extension)
{
	char *src;
//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != PATHSEPERATOR && src != path)
	{
		if (*src == EXTSEPERATOR)
			return;                 // it has an extension
		src--;
	}

	strcat (path, extension);
}

void DefaultPath (char *path, char *basepath)
{
	char    temp[128];

	if (path[0] == PATHSEPERATOR)
		return;				// absolute path location
	strcpy (temp, path);
	strcpy (path, basepath);
	strcat (path, temp);
}

void StripFilename (char *path)
{
	int length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != PATHSEPERATOR)
		length--;
	path[length] = 0;
}

void StripExtension (char *path)
{
	int length;

	length = strlen(path)-1;
	while (length > 0 && path[length] != '.')
		length--;
	if (length)
		path[length] = 0;
}

void ExtractFileBase (char *path, char *dest)
{
	char	*src;
	int	length;

	src = path + strlen(path) - 1;

//
// back up until a \ or the start
//
	while (src != path && *(src-1) != PATHSEPERATOR)
		src--;

//
// copy up to eight characters
//
	memset (dest, 0, 8);
	length = 0;
	while (*src && *src != '.')
	{
		if (++length == 9)
			Error ("Filename base of %s >8 chars", path);
		*dest++ = toupper(*src++);
	}
}

/*
==============
=
= ParseNum / ParseHex
=
==============
*/

long ParseHex (char *hex)
{
	char	*str;
	int	num;

	num = 0;
	str = hex;

	while (*str)
	{
		num <<= 4;
		if (*str >= '0' && *str <= '9')
			num += *str-'0';
		else if (*str >= 'a' && *str <= 'f')
			num += 10 + *str-'a';
		else if (*str >= 'A' && *str <= 'F')
			num += 10 + *str-'A';
		else
			Error ("Bad hex number: %s", hex);
		str++;
	}

	return num;
}

long ParseNum (char *str)
{
#ifdef RISCOS
	if (str[0] == '$' || str[0] == '&')
#else
	if (str[0] == '$')
#endif
		return ParseHex (str+1);
	if (str[0] == '0' && str[1] == 'x')
		return ParseHex (str+2);
	return atol (str);
}

int GetKey (void)
{
#if defined(UNIX) || defined(RISCOS)
        return getchar ();
#else
	return _bios_keybrd (_KEYBRD_READ)&0xff;
#endif
}
