/*******************************************************************
 * File:     unixlike.h
 * Purpose:  A unixlike library for RISC OS - specifically for libwad
 * Author:   Justin Fletcher
 * Date:     11 Oct 1998
 ******************************************************************/

#ifndef UNIXLIKE_H
#define UNIXLIKE_H

/* #include <swis.h> */
/* #include <kernel.h> */

#define O_RDONLY (1)
#define O_RDWR   (3)
#define O_BINARY (4)
#define O_TRUNC  (8)
#define O_CREAT  (16)

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */

/* Open a file */
int open(char *file,int type,...);

/* Read some data */
int read(int fd,void *buf, int length);

/* Write some data */
int write(int fd,void *buf, int length);

/* Close a file */
int close(int fd);

/* How long is this file */
int filelength (int fd);

/* Where are we in it ? */
int tell (int fd);

/* goto? */
int lseek (int fd,int pos, int from);

/* Set the type of a file */
/* Not very unixlike, but incredibly useful */
int settype(char *file, int type);

#endif
