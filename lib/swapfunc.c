/******************************************************************************/
/*Copyright (C) 2000 by Oliver Kraus (kraus@stanley.e-technik.uni-erlangen.de)*/
/*                                                                            */
/*      Permission to use, copy, modify, and distribute this software         */
/*      and its documentation for any purpose and without fee is              */
/*      hereby granted, provided that the above copyright notice              */
/*      appears in all copies and that both that copyright notice and         */
/*      this permission notice appear in supporting documentation.            */
/*      The author and contibutors make no representations about the          */
/*      suitability of this software for any purpose. It is provided          */
/*      "as is" without express or implied warranty.                          */
/******************************************************************************/

#include <stdio.h>
#include "wad.h"
#include "sysdep.h"

void swap_lump_p(void *l)
{
  swaplong(&(((filelump_t *)l)->filepos));
  swaplong(&(((filelump_t *)l)->size));
}

void swap_lump_list(void *l, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_lump_p( ((filelump_t *)l) + i );
}

void swap_thing_p(void *t)
{
  swapint(&(((things_t *)(t))->x));
  swapint(&(((things_t *)(t))->y));
  swapint(&(((things_t *)(t))->angle));
  swapint(&(((things_t *)(t))->type));
  swapint(&(((things_t *)(t))->flags));
}

void swap_thing_list(void *t, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_thing_p( ((things_t *)t) + i );
}

void swap_sidedef_p(void *s)
{
  swapint(&(((sidedef_t *)(s))->x));
  swapint(&(((sidedef_t *)(s))->y));
  swapint(&(((sidedef_t *)(s))->sector));
}

void swap_sidedef_list(void *s, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_sidedef_p( ((sidedef_t *)s) + i );
}

void swap_linedef_p(void *l)
{
  swapint(&(((linedef_t *)(l))->v_from));
  swapint(&(((linedef_t *)(l))->v_to));
  swapint(&(((linedef_t *)(l))->flags));
  swapint(&(((linedef_t *)(l))->type)); 
  swapint(&(((linedef_t *)(l))->tag)); 
  swapint(&(((linedef_t *)(l))->r_side)); 
  swapint(&(((linedef_t *)(l))->l_side)); 
}

void swap_linedef_list(void *l, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_linedef_p( ((linedef_t *)l) + i );
}

void swap_vertex_p(void *v)
{
  swapint(&(((vertex_t *)v)->x));
  swapint(&(((vertex_t *)v)->y));
}

void swap_vertex_list(void *v, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_vertex_p( ((vertex_t *)v) + i );
}

void swap_sector_p(void *s)
{
  swapint(&(((sector_t *)(s))->f_height));
  swapint(&(((sector_t *)(s))->c_height));
  swapint(&(((sector_t *)(s))->light));
  swapint(&(((sector_t *)(s))->flags));
  swapint(&(((sector_t *)(s))->tag));
}

void swap_sector_list(void *s, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_sector_p( ((sector_t *)s) + i );
}

#pragma pack(2)

typedef struct {
	short	numsegs;
	short	firstseg;	/* segs are stored sequentially */
} swap_func_subsector_t;

#pragma pack()

void swap_subsector_p(void *s)
{
  swapint(&(((swap_func_subsector_t *)s)->numsegs));
  swapint(&(((swap_func_subsector_t *)s)->firstseg));
}

void swap_subsector_list(void *s, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_subsector_p( ((swap_func_subsector_t *)s) + i );
}

#pragma pack(2)

typedef struct {
	short	v1,
		v2;
	short	angle;
	short	linedef,
		side;
	short	offset;
} swap_func_seg_t;

#pragma pack()

void swap_seg_p(void *s)
{
  swapint(&(((swap_func_seg_t *)(s))->v1)); 
  swapint(&(((swap_func_seg_t *)(s))->v2)); 
  swapint(&(((swap_func_seg_t *)(s))->angle)); 
  swapint(&(((swap_func_seg_t *)(s))->linedef)); 
  swapint(&(((swap_func_seg_t *)(s))->side)); 
  swapint(&(((swap_func_seg_t *)(s))->offset)); 
}

void swap_seg_list(void *s, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_seg_p( ((swap_func_seg_t *)s) + i );
}

#pragma pack(2)

typedef struct {
	short		x,
			y,
			dx,
			dy;		
	short		bbox[2][4];	
	unsigned short	children[2];	
} swap_func_node_t;

#pragma pack()

void swap_node_p(void *s)
{
  swapint(&(((swap_func_node_t *)(s))->x));
  swapint(&(((swap_func_node_t *)(s))->y));
  swapint(&(((swap_func_node_t *)(s))->dx));
  swapint(&(((swap_func_node_t *)(s))->dy));
  swapint(&(((swap_func_node_t *)(s))->bbox[0][0]));
  swapint(&(((swap_func_node_t *)(s))->bbox[0][1]));
  swapint(&(((swap_func_node_t *)(s))->bbox[0][2]));
  swapint(&(((swap_func_node_t *)(s))->bbox[0][3]));
  swapint(&(((swap_func_node_t *)(s))->bbox[1][0]));
  swapint(&(((swap_func_node_t *)(s))->bbox[1][1]));
  swapint(&(((swap_func_node_t *)(s))->bbox[1][2]));
  swapint(&(((swap_func_node_t *)(s))->bbox[1][3]));
  swapint(&(((swap_func_node_t *)(s))->children[0]));
  swapint(&(((swap_func_node_t *)(s))->children[1]));
}

void swap_node_list(void *s, int n)
{
  int i;
  for( i = 0; i < n; i++ )
    swap_node_p( ((swap_func_node_t *)s) + i );
}
