/*
 *	mygetopt.h
 *	POSIX getopt()
 *
 *	This module is a modified version of getopt.h from
 *	ftp://ftp.gnu.org/pub/gnu/glibc/glibc-2.1.2.tar.gz.
 *	What I did was :
 *
 *	1. prefix all external identifiers by "my_", to avoid
 *	   collision with any getopt() that might exist in libc,
 *
 *	2. remove things that were only needed used when
 *	   compiling for glibc,
 *
 *	3. remove things that were only needed to support long
 *	   options,
 *
 *	4. remove things that were only needed to support
 *	   obscure systems where strchr() is not available, etc.
 *
 *	AYM 1999-10-25
 */

/* Declarations for getopt.
   Copyright (C) 1989,90,91,92,93,94,96,97,98 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _GETOPT_H

#ifndef __need_getopt
# define _GETOPT_H 1
#endif

#ifdef	__cplusplus
extern "C" {
#endif

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

extern char *my_optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

extern int my_optind;

/* Callers store zero here to inhibit the error message `getopt' prints
   for unrecognized options.  */

extern int my_opterr;

/* Set to an option character which was unrecognized.  */

extern int my_optopt;

/* Get definitions and prototypes for functions to process the
   arguments in ARGV (ARGC of them, minus the program name) for
   options given in OPTS.

   Return the option character from OPTS just read.  Return -1 when
   there are no more options.  For unrecognized options, or options
   missing arguments, `optopt' is set to the option letter, and '?' is
   returned.

   The OPTS string is a list of characters which are recognized option
   letters, optionally followed by colons, specifying that that letter
   takes an argument, to be placed in `optarg'.

   If a letter in OPTS is followed by two colons, its argument is
   optional.  This behavior is specific to the GNU `getopt'.

   The argument `--' causes premature termination of argument
   scanning, explicitly telling `getopt' that there are no more
   options.

   If OPTS begins with `--', then non-option arguments are treated as
   arguments to the option '\0'.  This behavior is specific to the GNU
   `getopt'.  */

extern int my_getopt (int __argc, char *const *__argv, const char *__shortopts);

#ifdef	__cplusplus
}
#endif

#endif /* getopt.h */
