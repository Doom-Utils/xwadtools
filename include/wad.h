/************************************************************************/
/*      Copyright (C) 1998, 1999, 2000 by Udo Munk (munkudo@aol.com)    */
/*                                                                      */
/*      Permission to use, copy, modify, and distribute this software   */
/*      and its documentation for any purpose and without fee is        */
/*      hereby granted, provided that the above copyright notice        */
/*      appears in all copies and that both that copyright notice and   */
/*      this permission notice appear in supporting documentation.      */
/*      The author and contibutors make no representations about the    */
/*      suitability of this software for any purpose. It is provided    */
/*      "as is" without express or implied warranty.                    */
/************************************************************************/

/*
 * This include file describes the structures of WAD files.
 * The structures are documented in UDS and Official Hexen Specs,
 * and Doom and Hexen sources have been examined to make sure they
 * are correct.
 */

#ifndef __WAD_H__
#define __WAD_H__

#pragma pack(2)

/* Structure of the WAD header */
typedef struct
{
    char                identification[4];	/* string IWAD or PWAD */
    int                 numlumps;		/* number of lumps in WAD */
    int                 infotableofs;		/* directory offset */
} wadinfo_t;

/* Structure of the lump directory entries */
typedef struct
{
    int                 filepos;		/* lump offset */
    int                 size;			/* lump size */
    char                name[8];		/* lump name, padded with 0 */
} filelump_t;

/* Type of WAD file */
typedef enum
{
    PWAD,
    IWAD
} wad_t;

/*
 * Structure of the vertices in the VERTEXES lump for level maps
 */
typedef struct
{
    short		x;			/* x coordinate of vertex */
    short		y;			/* y coordinate of vertex */
} vertex_t;

/*
 * Structure of the Doom linedefs in the LINEDEFS lump for level maps
 */
typedef struct
{
    short		v_from;			/* from vertex */
    short		v_to;			/* to vertex */
    short		flags;			/* flags */
    short		type;			/* type */
    short		tag;			/* tag */
    short		r_side;			/* number of right sidedef */
    short		l_side;			/* number of left sidedef */
} linedef_t;

/*
 * Structure of the Hexen linedefs in the LINEDEFS lump for level maps
 */
typedef struct
{
    short		v_from;			/* from vertex */
    short		v_to;			/* to vertex */
    short		flags;			/* flags */
    unsigned char	type;			/* type */
    unsigned char	arg1;			/* linedef arguments */
    unsigned char	arg2;
    unsigned char	arg3;
    unsigned char	arg4;
    unsigned char	arg5;
    short		r_side;			/* number of right sidedef */
    short		l_side;			/* number of left sidedef */
} linedef2_t;

/*
 * Structure of the sidedefs in the SIDEDEFS lump for level maps
 */
typedef struct
{
    short		x;			/* x offset for texture */
    short		y;			/* y offset for texture */
    char		u_texture[8];		/* name upper texture */
    char		l_texture[8];		/* name lower texture */
    char		m_texture[8];		/* name middle texture */
    short		sector;			/* no. sector facing the sdef */
} sidedef_t;

/*
 * Structure of the sectors in the SECTORS lump for level maps
 */
typedef struct
{
    short		f_height;		/* floor height in sector */
    short		c_height;		/* ceiling height in sector */
    char		f_texture[8];		/* name of floor texture */
    char		c_texture[8];		/* name of ceiling texture */
    short		light;			/* light level in sector */
    short		flags;			/* flags */
    short		tag;			/* tag number */
} sector_t;

/*
 * Structure of the Doom things in the THINGS lump for level maps
 */
typedef struct
{
    short		x;			/* x coordinate of thing */
    short		y;			/* y coordinate of thing */
    short		angle;			/* angle thing faces */
    short		type;			/* type of thing */
    short		flags;			/* flags */
} things_t;

/*
 * Structure of the Hexen things in the THINGS lump for level maps
 */
typedef struct
{
    unsigned short	thingid;		/* thing id */
    short		x;			/* x coordinate of thing */
    short		y;			/* y coordinate of thing */
    short		z;			/* height of thing */
    short		angle;			/* angle thing faces */
    short		type;			/* type of thing */
    short		flags;			/* flags */
    unsigned char	special;		/* special to execute */
    unsigned char	arg1;			/* arguments for special */
    unsigned char	arg2;
    unsigned char	arg3;
    unsigned char	arg4;
    unsigned char	arg5;
} things2_t;

#pragma pack()

#endif
