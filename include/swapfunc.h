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


#ifndef __SWAPFUNC_H
#define __SWAPFUNC_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef BIGEND
#ifdef __sparc
#define BIGEND
#endif
#endif

#ifdef BIGEND

void swap_lump_p(void *l);
void swap_lump_list(void *l, int n);
void swap_thing_p(void *t);
void swap_thing_list(void *t, int n);
void swap_sidedef_p(void *s);
void swap_sidedef_list(void *s, int n);
void swap_linedef_p(void *l);
void swap_linedef_list(void *l, int n);
void swap_vertex_p(void *v);
void swap_vertex_list(void *v, int n);
void swap_sector_p(void *s);
void swap_sector_list(void *s, int n);
void swap_subsector_p(void *s);
void swap_subsector_list(void *s, int n);
void swap_seg_p(void *s);
void swap_seg_list(void *s, int n);
void swap_node_p(void *s);
void swap_node_list(void *s, int n);

#else

#define swap_lump_p(l)
#define swap_lump_list(l,n)

#define swap_thing_p(t)
#define swap_thing_list(t,n)

#define swap_sidedef_p(s)
#define swap_sidedef_list(s,n)

#define swap_linedef_p(l)
#define swap_linedef_list(l,n)

#define swap_vertex_p(v)
#define swap_vertex_list(v,n)

#define swap_sector_p(s)
#define swap_sector_list(s,n)

#define swap_subsector_p(s)
#define swap_subsector_list(s,n)

#define swap_seg_p(s)
#define swap_seg_list(s,n)

#define swap_node_p(s)
#define swap_node_list(s,n)

#endif /* BIGEND */

#ifdef	__cplusplus
}
#endif


#endif /* __SWAPFUNC_H */
