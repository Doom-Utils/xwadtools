/*- PICKNODE.C --------------------------------------------------------------*
 To be able to divide the nodes down, this routine must decide which is the
 best Seg to use as a nodeline. It does this by selecting the line with least
 splits and has least difference of Segs on either side of it.

 Credit to Raphael Quinet and DEU, this routine is a copy of the nodeline
 picker used in DEU5beta. I am using this method because the method I
 originally used was not so good.

 Rewritten by Lee Killough to significantly improve performance, while
 not affecting results one bit in >99% of cases (some tiny differences
 due to roundoff error may occur, but they are insignificant).
*---------------------------------------------------------------------------*/

#define FACTOR 8  /* This is the original "factor" used by previous versions
                     of the code -- it must be maintained in a macro to avoid
                     mistakes if we are to keep the tradition of using it,
                     and being able to modify it.
		   */

static int factor=2*FACTOR+1;

static struct Seg *PickNode_traditional(struct Seg *ts)
{
 struct Seg *best=NULL;
 long bestcost=LONG_MAX;
 struct Seg *part;
 int cnt=0;

 for (part=ts;part;part=part->next) /* Count once and for all */
   cnt++;

 for (part=ts;part;part = part->next)	/* Use each Seg as partition*/
  {
   struct Seg *check;
   long cost=0,tot=0,diff=cnt;

   progress();           	        /* Something for the user to look at.*/

   for (check=ts;check;check=check->next) /* Check partition against all Segs*/
    {         /*     get state of lines' relation to each other    */
     long a = part->pdy * check->psx - part->pdx * check->psy + part->ptmp;
     long b = part->pdy * check->pex - part->pdx * check->pey + part->ptmp;
     if ((a^b) < 0)
       if (a && b)
	{                    /* Line is split; a,b nonzero, opposite sign */
         long l=check->len;
         long d=(l*a)/(a-b); /* Distance from start of intersection */
         if (d>=2)
          {
        /* If the linedef associated with this seg has a sector tag >= 900,
           treat it as precious; i.e. don't split it unless all other options
           are exhausted. This is used to protect deep water and invisible
           lifts/stairs from being messed up accidentally by splits. */

           if (linedefs[check->linedef].tag >= 900)
	     cost += factor*64;

           cost += factor;

           if (cost > bestcost)     /* This is the heart of my pruning idea - */
	     goto prune;            /* it catches bad segs early on. Killough */

           tot++;
          }
         else
          if (l-d<2 ? check->pdx*part->pdx+check->pdy*part->pdy<0 : b<0)
            goto leftside;
        }
       else
         goto leftside;
     else
       if (a<=0 && (a || (!b && check->pdx*part->pdx+check->pdy*part->pdy<0)))
        {
         leftside:
           diff-=2;
        }
    }

   if ((diff-=tot) < 0) /* Take absolute value. diff is being used to obtain the */
     diff= -diff;       /* min/max values by way of: min(a,b)=(a+b-abs(a-b))/2   */

   /* Make sure at least one Seg is on each side of the partition*/

   if (tot+cnt > diff && (cost+=diff) < bestcost)
    {                   /* We have a new better choice */
     bestcost = cost;
     best = part;   	/* Remember which Seg*/
    }
   prune:;              /* Early exit and skip past the tests above */
  }
 return best;		/* All finished, return best Seg*/
}


/* Lee Killough 06/1997:

   The chances of visplane overflows can be reduced by attemping to
   balance the number of distinct sector references (as opposed to
   SEGS) that are on each side of the node line, and by rejecting
   node lines that cut across wide open space, as measured by the
   proportion of the node line which is incident with segs, inside
   the bounding box.

   Node lines which are extensions of linedefs whose vertices are
   on the boundary of the bounding box, are therefore preferable,
   as long as the number of sectors referenced on either side is
   not too unbalanced.

   Contrary to what many say, visplane overflows are not simply
   caused by too many sidedefs, linedefs, light levels, etc. All
   of those factors are correlated with visplane overflows, but
   most importantly, so is how the node builder selects node
   lines. The number of visible changes in flats is the main
   cause of visplane overflows, with visible changes not being
   counted if only invisible regions separate the visible areas.
*/

static struct Seg *PickNode_visplane(struct Seg *ts)
{
 struct Seg *best=NULL;
 long bestcost=LONG_MAX;
 struct Seg *part;
 int cnt=0;

 for (part=ts;part;part=part->next)     /* Count once and for all */
   cnt++;

 FindLimits(ts);

 for (part=ts;part;part=part->next)	/* Use each Seg as partition*/
  {
   struct Seg *check;
   long cost=0,slen=0;
   int tot=0,diff=cnt;
   memset(SectorHits,0,num_sects);
   progress();           	        /* Something for the user to look at.*/

   for (check=ts;check;check=check->next) /* Check partition against all Segs*/
    {
        /*     get state of lines' relation to each other    */
     long a = part->pdy * check->psx - part->pdx * check->psy + part->ptmp;
     long b = part->pdy * check->pex - part->pdx * check->pey + part->ptmp;
     unsigned char mask=2;

     if ((a^b) < 0)
       if (a && b)
	{                /* Line is split; a,b nonzero, opposite sign */
         long l=check->len;
         long d=(l*a)/(a-b);    /* Distance from start of intersection */
         if (d>=2)
          {
        /* If the linedef associated with this seg has a sector tag >= 900,
           treat it as precious; i.e. don't split it unless all other options
           are exhausted. This is used to protect deep water and invisible
           lifts/stairs from being messed up accidentally by splits. */

           if (linedefs[check->linedef].tag >= 900)
	     cost += factor*64;

           cost += factor;

	   if (cost > bestcost)     /* This is the heart of my pruning idea - */
	     goto prune;            /* it catches bad segs early on. Killough */

           tot++;               /* Seg is clearly split */
           mask=4;
          }
         else         /* Distance from start < 2; check distance from end */
           if (l-d<2 ? check->pdx*part->pdx+check->pdy*part->pdy<0 : b<0)
             goto leftside;
        }
       else
         goto leftside;
     else
       if (a<=0 && (a || (!b && (slen+=check->len,
                     check->pdx*part->pdx+check->pdy*part->pdy<0))))
        {
         leftside:
           diff-=2;
           mask=1;
        }
     SectorHits[check->sector] |= mask;
    }

   if ((diff-=tot) < 0) /* Take absolute value. diff is being used to obtain the */
     diff= -diff;       /* min/max values by way of: min(a,b)=(a+b-abs(a-b))/2   */

   /* Make sure at least one Seg is on each side of the partition*/

   if (tot+cnt <= diff)
     continue;

   /* Compute difference in number of sector
      references on each side of node line */

   for (diff=tot=0;tot<num_sects;tot++)
     switch (SectorHits[tot])
      {
       case 1:
         diff++;
         break;
       case 2:
         diff--;
         break;
      }

   if (diff<0)
     diff= -diff;

   if ((cost+=diff) >= bestcost)
     continue;

   /* If the node line is incident with SEGS in less than 1/2th of its
      length inside the bounding box, increase the cost since this is
      likely a node line cutting across a large room but only sharing
      space with a tiny SEG in the middle -- this is another contributor
      to visplane overflows. */

      {
       long l;
       if (!part->pdx)
         l=lmaxy-lminy;
       else
        {
         if (!part->pdy)
           l=lmaxx-lminx;
         else
          {
           double t1=(part->psx-lmaxx)/(double) part->pdx;
           double t2=(part->psx-lminx)/(double) part->pdx;
           double t3=(part->psy-lmaxy)/(double) part->pdy;
           double t4=(part->psy-lminy)/(double) part->pdy;
           if (part->pdx>0)
            {
             double t=t1;
             t1=t2;
             t2=t;
            }
           if (part->pdy>0)
            {
             double t=t3;
             t3=t4;
             t4=t;
            }
           l=((t1 > t3 ? t3 : t1) - (t2 < t4 ? t4 : t2))*part->len;
          }
         if (slen < l && (cost+=factor) >= bestcost)
           continue;
        }
      }

   /* We have a new better choice */

   bestcost = cost;
   best = part;   	/* Remember which Seg*/
   prune:;              /* Early exit and skip past the tests above */
  }
 return best;		/* All finished, return best Seg*/
}

struct halfplane {
  long pdx,pdy,ptmp;
};

struct triangle {
  long pdx1,pdy1,ptmp1;
  long pdx2,pdy2,ptmp2;
  long pdx3,pdy3,ptmp3;
};

struct polygon {
 struct triangle tri;
 long pdx1,pdy1,pdz1,ptmp1;
 long pdx2,pdy2,pdz2,ptmp2;
};

struct view {
  long pdx,pdy,ptmp;      /* Used for 90 degree vertical view */
  double distr;
  long x,y,z;             /* Position */
  unsigned angle;         /* Angle 0-65535 */
  const struct Pseg *ps;  /* Seg the player is looking from */
  int sector;             /* Sector player is in */
  void (*func)(int, struct view *); /* Function used in BSP traversal */
  int polarity;           /* 1=back-to-front, 0=front-to-back traversal */
  struct triangle *poly;  /* Polygon limiting all visible areas */
  int num_poly,num_poly_alloc;
  struct halfplane *convex_hull; /* Convex hull limiting all visible areas */
  int num_convex_hull,num_convex_hull_alloc;
  struct polygon *poly_3d;    /* Polygon limiting floors & ceilings */
  int num_poly_3d,num_poly_3d_alloc;
  unsigned char *hitlist;     /* Used in searching algorithms */
  int visplanes;              /* Visplane count */
  int max_visplanes,max_visplanes_linedef,max_visplanes_sector;
  int start_markers;
};

static struct sector_limit {
  struct {
    long min,max;
    short moves;
  } floorh,ceilh;
} *sector_limits;

static int light_changes(int type)
{
 return (type>=1 && type<=4) || type==8 || type==12 || type==13 || type==17;
}

/* Whether two sectors' lighting is similar.

   Note: I've observed that even though lighting can only
   be seen in multiples of 8, visplane overflows can be
   caused by any difference in lighting, even if it's not
   visible. killough */

static int lighting_similar(const struct Sector *s1, const struct Sector *s2)
{
 return
 s1==s2 ||
 (            /* If initial lighting same, and tags are same */
  s1->light==s2->light && s1->tag==s2->tag
   &&
    (         /* And both are the same synchronized lighting */
     (s1->special==s2->special && (s1->special==12 || s1->special==13))
      ||      /* Or neither one changes in lighting based on sector type */
     (!light_changes(s1->special) && !light_changes(s2->special))
    )
 )
 ;
}

static int floor_similar(const struct Sector *s1, const struct Sector *s2)
{
 return s1==s2 ||
  (!strncmp(s1->floort,s2->floort,8) && (!strncmp(s1->floort,"F_SKY1",8) ||
  (s1->floorh==s2->floorh && (s1->tag==s2->tag ||
  (!sector_limits[s1-sectors].floorh.moves &&
   !sector_limits[s2-sectors].floorh.moves )))));
}

static int ceil_similar(const struct Sector *s1, const struct Sector *s2)
{
 return s1==s2 ||
  (!strncmp(s1->ceilt,s2->ceilt,8) && (!strncmp(s1->ceilt,"F_SKY1",8) ||
  (s1->ceilh==s2->ceilh && (s1->tag==s2->tag ||
  (!sector_limits[s1-sectors].ceilh.moves &&
   !sector_limits[s2-sectors].ceilh.moves )))));
}

/* Find blocking linedefs and create polygons representing view.

   This is perhaps the most important function. For each ssector
   whose bounding box is contained within the convex hull of the
   visibilty region, all segs in the ssector are examined and
   any segs that face the viewer have their linedefs analyzed
   for visibility-blocking properties.

   4 structures are maintained for the view:

   A 2-d convex hull of the visible region
   A set of 2-d triangles for 1s linedefs blocking visibility
   A set of halfplanes for 2s linedefs blocking visibility
   Two 3-d halfplanes limiting the player's vertical visibility

   Having these separate structures allows greater
   optimization due to the 2-d nature of Doom.
*/

static void add_blocking_linedefs(int ss, struct view *v)
{
 const struct Pseg *ps=psegs+ssectors[ss].first;
 int num;

 /* Go through every seg in the ssector and if the seg is facing
    at all towards the player, and if its associated linedef has
    not been seen before, process the linedef for its effect on
    the view.

    This is more complicated, but faster, than going through all
    linedefs once, since it allows us to skip entire ssectors and
    nodes which are not in the player's view.
 */

 for (num=ssectors[ss].num;num--;ps++)
   if (!v->hitlist[ps->linedef])
    {
     const struct LineDef *l = ps->linedef+linedefs;
     long psx=vertices[ps->flip ? l->end : l->start].x;
     long psy=vertices[ps->flip ? l->end : l->start].y;
     long pex=vertices[ps->flip ? l->start : l->end].x;
     long pey=vertices[ps->flip ? l->start : l->end].y;
     long pdx=pex-psx,pdy=pey-psy;

     if ((psx-v->x)*pdy < (psy-v->y)*pdx)
      {                /* Only consider segs facing towards player */
       struct triangle *tri;

       v->hitlist[ps->linedef]=1;  /* Mark linedef as having been seen */

    /* If a linedef does not have the 2s flag set, then regardless of
       whether it has a second sidedef, it cannot be seen through. It
       may be shot through with projectiles (rockets, plasma, monster
       fire), but not bullets. Monsters cannot see through it either,
       and will not intentionally attack through it, although they can
       walk and hear through it, and fire through it if they fire at a
       thing they can see which is in the same direction. killough */

       if (!(l->flags & 4))
        {
         struct halfplane *ch;
         int num,start_is_out;

onesided:
         num=v->num_convex_hull;
         start_is_out=0;
         ch=v->convex_hull;

    /* If the starting and ending vertices are both outside any single
       halfplane, ignore the line completely. */

         do
           if (ch->pdx*psy - ch->pdy*psx > ch->ptmp)   /* Start */
	    {
             if (ch->pdx*pey - ch->pdy*pex > ch->ptmp) /* End */
               goto skip;
             else
               start_is_out=1;     /* Mark starting vertex as outside */
	    }
         while (ch++,--num);

    /* If the starting and vertices are both outside of the convex hull,
       but are not both outside of any single halfplane, then add the
       line to the convex hull. */

         if (start_is_out)
          { /* Starting vertex is outside convex hull; check ending vertex */
           ch=v->convex_hull;
           num=v->num_convex_hull;
           do                /* If ending vertex is outside of convex hull */
             if (ch->pdx*pey - ch->pdy*pex > ch->ptmp)
              {             /* Add the line to convex hull */

    /* Add the extension of the linedef to the convex hull.
       If it is totally outside of the convex hull, it has
       no effect since the viewer is always inside its
       halfplane. If it cuts across the convex hull, then
       it restricts the visible area some more. */

               if (v->num_convex_hull >= v->num_convex_hull_alloc)
                   v->convex_hull = ResizeMemory(v->convex_hull,
                   (v->num_convex_hull_alloc*=2)*sizeof(*v->convex_hull));
               ch=v->convex_hull+v->num_convex_hull++;
               ch->ptmp=(ch->pdx=pdx)*psy-(ch->pdy=pdy)*psx;
               goto skip;    /* Skip to next seg */
              }
           while (ch++,--num);
          }

    /* At least one vertex is inside convex hull; add
       triangle between viewer and linedef to polygon
       (eye clipping could be used, to remove parts of
       the triangle that have no effect, but I prefer
       to stay simple). */

         if (v->num_poly >= v->num_poly_alloc)
           v->poly = ResizeMemory(v->poly,
                    (v->num_poly_alloc=v->num_poly_alloc*2+2)*
                      sizeof(*v->poly));
         tri = v->poly+v->num_poly++;
        }
       else /* Line is 2s, possibly see-through depending on sector heights */
        {
         int s1 = sidedefs[ps->flip ? l->sidedef2 : l->sidedef1].sector; /* front */
         int s2 = sidedefs[ps->flip ? l->sidedef1 : l->sidedef2].sector; /* back  */
         const long *lower_blocks=NULL, *upper_blocks=NULL;

         if (s1==s2)     /* Skip linedefs with the same sector on both sides */
           goto skip;

         if (sector_limits[s2].floorh.min >= sector_limits[s2].ceilh.max)
           goto onesided; /* If floor always touches ceiling, treat as 1s */

         if (sector_limits[s1].floorh.min < sector_limits[s2].floorh.min)
           lower_blocks=&sector_limits[s2].floorh.min;

         if (sector_limits[s1].ceilh.max > sector_limits[s2].ceilh.max)
           upper_blocks=&sector_limits[s2].ceilh.max;

         if (!lower_blocks && !upper_blocks)
           goto skip;      /* Neither upper nor lower texture blocks */

    /* Allocate a new 3d polygon to represent the view halfplanes */

         if (v->num_poly_3d >= v->num_poly_3d_alloc)
           v->poly_3d = ResizeMemory(v->poly_3d,
                        (v->num_poly_3d_alloc=v->num_poly_3d_alloc*2+1) *
                           sizeof(*v->poly_3d));

         {
          struct polygon *p = v->poly_3d + v->num_poly_3d++;

          long t1,t2=(t1=v->x*pdy+(psy-v->y)*pex+(v->y-pey)*psx)*v->z;

    /* Where to put triangle between player and linedef (irrespective of z) */

          tri = &p->tri;

    /* Everything above triangle between the higher of the
       two sectors' floors and the viewer is unblocked */

          if (lower_blocks)
           {
            p->pdz1=t1;
            p->ptmp1=(p->pdx1=(*lower_blocks-v->z)*pdy)*v->x+
                     (p->pdy1=(v->z-*lower_blocks)*pdx)*v->y+t2;

    /* Everything below triangle between the lower of the
       two sectors' ceilings and the viewer is unblocked */

            if (upper_blocks)
             {
              p->pdz2= -t1;
              p->ptmp2=(p->pdx2=(v->z-*upper_blocks)*pdy)*v->x+
                       (p->pdy2=(*upper_blocks-v->z)*pdx)*v->y-t2;
             }
            else
              p->ptmp2=p->pdx2=p->pdy2=p->pdz2=0;
           }
          else         /* Lower texture does not block => upper blocks */
           {
    /* Everything below triangle between the lower of the
       two sectors' ceilings and the viewer is unblocked
       (put in first entry for performance) */

            p->pdz1= -t1;
            p->ptmp1=(p->pdx1=(v->z-*upper_blocks)*pdy)*v->x+
                     (p->pdy1=(*upper_blocks-v->z)*pdx)*v->y-t2;
            p->ptmp2=p->pdx2=p->pdy2=p->pdz2=0;
           }
         }
        }

    /* We form a triangle between the viewer and the linedef,
       to indicate anything behind it is blocked (unless it's
       in the visible area between two planes formed between
       the viewer and the floor and ceiling, in which case the
       linedef is 2s and this is part of a larger disjunction). */

    /* Everything to the right of the linedef is unblocked */

       tri->ptmp1 = (tri->pdx1=pdx     )*psy-(tri->pdy1=pdy     )*psx;

    /* Everything to the right of line between start and viewer is unblocked */

       tri->ptmp2 = (tri->pdx2=v->x-psx)*psy-(tri->pdy2=v->y-psy)*psx;

    /* Everything to the right of line between viewer and end is unblocked */

       tri->ptmp3 = (tri->pdx3=pex-v->x)*pey-(tri->pdy3=pey-v->y)*pex;
      }
skip:;
    }
}

enum {FLOOR=1, CEILING=2};

/* Whether a seg is visible in the current view, at the heights indicated
   by the sector. 0=totally invisible, 1=floor visible, 2=ceiling visible,
   3=both visible. Only the right side of a seg is considered, so if a
   seg is incident with a closed halfplane, its right side must face
   into the halfplane.

   Note: this is just an approximation; the exact way to tell visibility
   is to compute the area of the intersection of the polygon and the
   triangle between the viewer and the seg, which is more difficult. */

static int seg_is_visible(const struct Pseg *ps,
                          const struct Sector *sector,
                          const struct view *v)
{
 long psx=vertices[ps->start].x;
 long psy=vertices[ps->start].y;
 long pex=vertices[ps->end].x;
 long pey=vertices[ps->end].y;
 long pdx=pex-psx,pdy=pey-psy;            /* (dx,dy) vector */

 if ((psx-v->x)*pdy >= (psy-v->y)*pdx)  /* backface cull */
   return 0;
 else
  {
   int floorh=sector->floorh, ceilh=sector->ceilh;
   int floor_blocked, ceil_blocked;

  /* Player's 90-degree vertical camera angle.
     Limits how far up and down a player can see. */

   {
    long s=v->pdy*psx-v->pdx*psy-v->ptmp;
    long e=v->pdy*pex-v->pdx*pey-v->ptmp;
    double t;
    if (e>s)
      s=e;
    if (s<0)
      return 0;
    t=s*v->distr;
    floor_blocked = (e=v->z-floorh) > t || -e > t;
    ceil_blocked  = (e=v->z-ceilh)  > t || -e > t;
    if (floor_blocked && ceil_blocked)
      return 0;
   }

  /* If the starting or ending vertices are inside a 2-halfplane,
     then the seg is at least partially inside the 2-halfplane.
     If they are both incident to the 2-halfplane then one side
     of the seg is inside the 2-halfplane and the other is not,
     as determined by the dot product. Otherwise (if one vertex
     is outside of the 2-halfplane and the other is outside or
     incident to it), the seg is totally outside of the
     2-halfplane. We are only interested in whether the right
     side is in the 2-halfplane. */

   {
    struct halfplane *ch=v->convex_hull;
    int i=v->num_convex_hull;
    do
     {
      long s,e;
      if ( (s=ch->pdx*psy-ch->pdy*psx-ch->ptmp) >=0 &&
           (e=ch->pdx*pey-ch->pdy*pex-ch->ptmp) >=0 &&
           (e || s || ch->pdx*pdx+ch->pdy*pdy<0) )
        return 0;
      ch++;
     }
    while (--i);
   }

   /* Same as above, except that each triangle is an union
      (OR) of three 2-halfplanes, and so if the seg is inside
      any one of them it is considered unblocked. */

   {
    int i=v->num_poly;
    if (i)
     {
      struct triangle *tri=v->poly;
      do
       {
        long s1,e1,s2,e2,s3,e3;
        if ( (s1=tri->pdx1*psy-tri->pdy1*psx-tri->ptmp1) >=0 &&
             (e1=tri->pdx1*pey-tri->pdy1*pex-tri->ptmp1) >=0 &&
             (s2=tri->pdx2*psy-tri->pdy2*psx-tri->ptmp2) >=0 &&
             (e2=tri->pdx2*pey-tri->pdy2*pex-tri->ptmp2) >=0 &&
             (s3=tri->pdx3*psy-tri->pdy3*psx-tri->ptmp3) >=0 &&
             (e3=tri->pdx3*pey-tri->pdy3*pex-tri->ptmp3) >=0 &&
             (e3 || s3 || tri->pdx3*pdx+tri->pdy3*pdy < 0) &&
             (e2 || s2 || tri->pdx2*pdx+tri->pdy2*pdy < 0) &&
             (e1 || s1 || tri->pdx1*pdx+tri->pdy1*pdy < 0) )
          return 0;
        tri++;
       }
      while (--i);
     }
   }

   {
    register struct polygon *p=v->poly_3d;
    int i;

    for (i=v->num_poly_3d;i--;p++)
     {
      long s1,e1,s2,e2,s3,e3;
      if ( (s1=p->tri.pdx1*psy-p->tri.pdy1*psx-p->tri.ptmp1) >= 0 &&
           (e1=p->tri.pdx1*pey-p->tri.pdy1*pex-p->tri.ptmp1) >= 0 &&
           (s2=p->tri.pdx2*psy-p->tri.pdy2*psx-p->tri.ptmp2) >= 0 &&
           (e2=p->tri.pdx2*pey-p->tri.pdy2*pex-p->tri.ptmp2) >= 0 &&
           (s3=p->tri.pdx3*psy-p->tri.pdy3*psx-p->tri.ptmp3) >= 0 &&
           (e3=p->tri.pdx3*pey-p->tri.pdy3*pex-p->tri.ptmp3) >= 0 &&
           (e3 || s3 || p->tri.pdx3*pdx+p->tri.pdy3*pdy < 0) &&
           (e2 || s2 || p->tri.pdx2*pdx+p->tri.pdy2*pdy < 0) &&
           (e1 || s1 || p->tri.pdx1*pdx+p->tri.pdy1*pdy < 0) )
       {
        long as=p->pdx1*psx+p->pdy1*psy-p->ptmp1;
        long ae=p->pdx1*pex+p->pdy1*pey-p->ptmp1;
        long bs=p->pdx2*psx+p->pdy2*psy-p->ptmp2;
        long be=p->pdx2*pex+p->pdy2*pey-p->ptmp2;
        long dotax=p->pdx1*pdy, dotay=p->pdy1*pdx;
        long dotbx=p->pdx2*pdy, dotby=p->pdy2*pdy;
        register long t;
        if (!floor_blocked && (((t=p->pdz1*floorh)+as<=0 && t+ae<=0 &&
                 (t+ae || t+as || dotax<dotay)) ||
            ((t=p->pdz2*floorh)+bs<=0 && t+be<=0 &&
                 (t+be || t+bs || dotbx<dotby)) ))
	 {
          if (ceil_blocked)
            return 0;
          else
            floor_blocked=1;
	 }
         if (!ceil_blocked && (((t=p->pdz1*ceilh)+as<=0 && t+ae<=0 &&
                  (t+ae || t+as || dotax<dotay)) ||
            ((t=p->pdz2*ceilh)+bs<=0 && t+be<=0 &&
                  (t+be || t+bs || dotbx<dotby)) ))
	  {
          if (floor_blocked)
            return 0;
          else
            ceil_blocked=1;
	  }
       }
     }
    return floor_blocked ? CEILING : ceil_blocked ? FLOOR : FLOOR | CEILING;
   }
  }
}

/* A bounding box is invisible if its four vertices are totally outside
   of any halfplane of the convex hull. When performing rendering, Doom
   skips nodes or ssectors whose bounding box is not inside the player's
   view. This is the main reason large levels can be rendered quickly in
   Doom with a single BSP tree. killough */

static int box_is_visible(int minx, int miny, int maxx, int maxy,
                          const struct view *v)
{
 int i=v->num_convex_hull;
 const struct halfplane *p=v->convex_hull;
 do
  {
   long t1,t2,t3;
   if ( (t1=p->pdx*maxy-p->ptmp) > (t2=p->pdy*minx) &&
                (t3=p->pdy*maxx) < t1 &&
        (t1=p->pdx*miny-p->ptmp) > t3 &&
                              t1 > t2)
     return FALSE;
   p++;
  }
 while (--i);
 return TRUE;
}

/* This is the main BSP traversal. We find the viewer's position
   by doing a binary search for the viewer's ssector. We process
   it first, and then go back up the BSP tree after processing
   the parent node's other child. Each time we reach a node, we
   decide whether to render the left child first or the right
   child first depending on the viewer's position w.r.t. that
   node's line. This is essentially an inorder traversal of the
   BSP tree. If the bounding box for a node or a ssector is
   totally invisible based on the (possibly dynamic) convex
   hull, we skip it, which is the main performance boost. killough */

static void bsp_traverse(const struct Node *tn, struct view *v)
{
 int lr=(tn->dx*v->y-tn->dy*v->x > tn->ptmp) ^ v->polarity;
 if (lr)
   goto leftside;
 do
  {
   if (box_is_visible(tn->minx1,tn->miny1,tn->maxx1,tn->maxy1,v))
    {
     if (tn->nextr)       /* Process only if bounding box is visible */
       bsp_traverse(tn->nextr,v);
     else                 /* Process right side */
       v->func(tn->chright,v);
    }
   if (lr) break;

leftside:
   if (box_is_visible(tn->minx2,tn->miny2,tn->maxx2,tn->maxy2,v))
    {
     if (tn->nextl)       /* Process only if bounding box is visible */
       bsp_traverse(tn->nextl,v);
     else                 /* Process left side */
       v->func(tn->chleft,v);
    }
  }
 while (lr);
}

/* We compute the player's view, limited by left and right side
   lines that are 90 degrees apart and centered at the player's
   angle, and by planes 45 degrees above and below. */

static int setup_view(struct view *v, const struct Pseg *ps, const double *adj)
{                                  /* Compute the player's view to be */
 long psx=vertices[ps->start].x;   /* Looking away from the seg, into */
 long psy=vertices[ps->start].y;   /* the corresponding ssector. */
 long pdx=vertices[ps->end].x-psx;
 long pdy=vertices[ps->end].y-psy;

 v->sector=sidedefs[ps->flip ? linedefs[ps->linedef].sidedef2 :
                               linedefs[ps->linedef].sidedef1].sector;

 v->ps=ps;

 v->x=(psx*2+pdx)/2;      /* Midpoint */
 v->y=(psy*2+pdy)/2;

 v->z=sectors[v->sector].floorh+48; /* Player sees 48 units above floor level */

 {
  long temp=pdx*adj[0]+pdy*adj[1];
  pdx=pdx*adj[1]-pdy*adj[0];
  pdy=temp;
 }
 if (!pdy && !pdx) return 0;
 v->angle=ComputeAngle(pdx,pdy);

 if (v->num_convex_hull_alloc<2)
  {
   v->num_convex_hull_alloc=2;
   v->convex_hull = ResizeMemory(v->convex_hull,2*sizeof(*v->convex_hull));
  }

 v->num_convex_hull=2;

 v->convex_hull[0].ptmp=(v->convex_hull[0].pdx=pdy+pdx)*v->y-
                        (v->convex_hull[0].pdy=pdy-pdx)*v->x;

 v->convex_hull[1].ptmp=(v->convex_hull[1].pdx=pdx-pdy)*v->y-
                        (v->convex_hull[1].pdy=pdy+pdx)*v->x;

 v->num_poly_3d=v->num_poly=0;

 v->ptmp=pdy*psx-pdx*psy;
 v->distr=1.618/sqrt((double) pdy*pdy + (double) pdx*pdx);
 v->pdx=pdx;
 v->pdy=pdy;

 /* No visplanes initially */
 v->visplanes=0;
 return 1;
}

static void R_DrawPlanes(int ss, struct view *v)
{
 const struct Pseg *ps=ssectors[ss].first+psegs;   /* First seg in ssector */

 /* The first seg in an ssector determines which sector describes its flats */

 const struct Sector *sector=sectors+sidedefs[ps->flip ?
                                              linedefs[ps->linedef].sidedef2 :
                                              linedefs[ps->linedef].sidedef1 ].sector;
 int num;

 for (num=ssectors[ss].num;num--;ps++)
  {
   if ((ps->angle-v->angle) & 0x8000) /* Backface cull the way Doom does it */
    {
     const struct LineDef *l=linedefs+ps->linedef; /* linedef this seg's along */
     if (l->flags & 4)          /* If linedef is 2s */
      {
       const struct Sector *s1=sectors+sidedefs[l->sidedef1].sector;
       const struct Sector *s2=sectors+sidedefs[l->sidedef2].sector;

       if (s1!=s2)                       /* If same sector, ignore */
        {
         unsigned vp_mask = ~(v->hitlist[s1-sectors] & v->hitlist[s2-sectors]) & (FLOOR | CEILING);

         if (vp_mask)
          {

      /* If a 2s linedef has identical floor or ceiling properties on
         either side of it, then a new triangle is not drawn for it
         (in the respective floor or ceiling), even if the two sectors
         on either side of it are not the same sector. This is well-known
         to those who have constructed invisible platforms or deep water.
         Note that neither of these two sectors is necessarily the one
         that governs flats drawing for this seg, if it is drawn at all --
         that is controlled by the sector of the first seg in the ssector. */

           if (lighting_similar(s1,s2))      /* If lighting is similar */
            {
             if (floor_similar(s1,s2))       /* If floor is similar */
               vp_mask &= ~FLOOR;            /* Then ignore floor */
             if (ceil_similar(s1,s2))        /* If ceiling is similar */
               vp_mask &= ~CEILING;          /* Then ignore ceiling */
            }

           if (vp_mask)
            {
             vp_mask &= seg_is_visible(ps, sector, v);
             v->hitlist[s1-sectors] |= vp_mask;
             v->hitlist[s2-sectors] |= vp_mask;
             if (vp_mask & FLOOR)
               v->visplanes++;
             if (vp_mask & CEILING)
               v->visplanes++;
            }
          }
        }
      }
    }
  }
}

static int visplanes(const struct Node *tn, const struct Pseg *ps, struct view *v)
{
 int maxvp=INT_MIN,i;
 double adj[][2]={{0,1},{-.7071067812,.7071067812},
                        {.7071067812,.7071067812}};

 for (i=0;i<sizeof adj/sizeof*adj;i++)
  {
   /* Initialize view structure and create initial convex hull */

   if (!setup_view(v,ps,adj[i])) continue;

   /* Memoize linedefs so that each one is analyzed
      only once during polygon construction */

   memset(v->hitlist, 0, num_lines);

   /* Form the polygons that define the current view */

   v->func=add_blocking_linedefs;
   v->polarity=0;         /* front-to-back */
   bsp_traverse(tn, v);

   /* Find the segs which are visible in the current view */
   v->polarity=1;         /* back-to-front */
   v->func=R_DrawPlanes;

   memset(v->hitlist, 0, num_sects);

   bsp_traverse(tn, v);

   if (v->visplanes > maxvp)
     maxvp=v->visplanes;
  }

 if (maxvp > v->max_visplanes)
  {
   v->max_visplanes=maxvp;
   v->max_visplanes_sector=v->sector;
   v->max_visplanes_linedef=v->ps->linedef;
  }

 return maxvp;
}


static void init_view(struct view *v)
{
 v->hitlist = GetMemory(num_sects > num_lines ? num_sects : num_lines);
 v->max_visplanes=INT_MIN;
 v->start_markers=num_things;
}

/* Clean up view. */

static void free_view(struct view *v)
{
 free(v->convex_hull);
 free(v->poly);
 free(v->poly_3d);
 free(v->hitlist);

 v->hitlist=NULL;
 v->convex_hull=NULL;
 v->poly=NULL;
 v->poly_3d=NULL;

 v->num_convex_hull=
 v->num_convex_hull_alloc=
 v->num_poly=
 v->num_poly_alloc=
 v->num_poly_3d=
 v->num_poly_3d_alloc=
 0;
}

static void raise_ceil(int s, int depth)
{
 int i;
 long l=LONG_MAX;
 sector_limits[s].ceilh.moves=1;
 for (i=0;i<num_lines;i++)
   if (linedefs[i].sidedef2!=-1)
    {
     int s2=sidedefs[linedefs[i].sidedef2].sector;
     int s1=sidedefs[linedefs[i].sidedef1].sector;
     if (s1!=s)
      {
       if (s2==s)
         s2=s1;
       else
         continue;
      }
     if (s2!=s && sectors[s2].ceilh<l)
       l=sectors[s2].ceilh;
    }
 if (l < LONG_MAX && (l-=depth) > sector_limits[s].ceilh.max)
   sector_limits[s].ceilh.max=l;
}

static void lower_floor(int s)
{
 int i;
 long l=LONG_MAX;
 sector_limits[s].floorh.moves=1;
 for (i=0;i<num_lines;i++)
   if (linedefs[i].sidedef2!=-1)
    {
     int s2=sidedefs[linedefs[i].sidedef2].sector;
     int s1=sidedefs[linedefs[i].sidedef1].sector;
     if (s1!=s)
      {
       if (s2==s)
         s2=s1;
       else
         continue;
      }
     if (s2!=s && sectors[s2].floorh<l)
       l=sectors[s2].floorh;
    }
 if (l < sector_limits[s].floorh.min)
   sector_limits[s].floorh.min=l;
}

static void lowern_floor(int s, int depth)
{
 int i;
 long l=LONG_MIN;
 sector_limits[s].floorh.moves=1;
 for (i=0;i<num_lines;i++)
   if (linedefs[i].sidedef2!=-1)
    {
     int s2=sidedefs[linedefs[i].sidedef2].sector;
     int s1=sidedefs[linedefs[i].sidedef1].sector;
     if (s1!=s)
      {
       if (s2==s)
         s2=s1;
       else
         continue;
      }
     if (s2!=s && sectors[s2].floorh>l)
       l=sectors[s2].floorh;
    }
 if (l > LONG_MIN && (l+=depth) < sector_limits[s].floorh.min)
   sector_limits[s].floorh.min=l;
}

static void build_stairs(int s, int step, int h)
{
 int k;

top:
 if ((h+=step) < sector_limits[s].floorh.min)
   sector_limits[s].floorh.min=h;
 sector_limits[s].floorh.moves=3;
 for (k=0;k<num_lines;k++)
   if (linedefs[k].sidedef2!=-1 && sidedefs[linedefs[k].sidedef1].sector==s)
    {
     if (!(sector_limits[k=sidedefs[linedefs[k].sidedef2].sector].floorh.moves & 2) &&
         !strncmp(sectors[k].floort,sectors[s].floort,8))
      {
       s=k;
       goto top;
      }
     else
       break;
    }
}

/* Simple one-pass algorithm; does not take into account multiple effects. */

static void find_sector_limits(void)
{
 int i,j;
 for (i=0;i<num_sects;i++)
  {
   sector_limits[i].ceilh.min=sector_limits[i].ceilh.max=sectors[i].ceilh;
   sector_limits[i].floorh.min=sector_limits[i].floorh.min=sectors[i].floorh;
   sector_limits[i].ceilh.moves=sector_limits[i].floorh.moves=0;
  }
 for (i=0;i<num_lines;i++)
   switch (linedefs[i].type)
    {
     case  1: case 26: case 28: case 27: case 31:
     case 32: case 33: case 34: case 46: case 117:
     case 118:
       if (linedefs[i].sidedef2 >= 0)    /* normal door open */
         raise_ceil(sidedefs[linedefs[i].sidedef2].sector,4);
       break;
     case 4: case 29: case 90: case 63:
     case 2: case 103: case 86: case 61: case 108:
     case 111: case 105: case 114: case 109: case 112:
     case 106: case 115: case 133: case 99: case 135:
     case 134: case 136: case 137: case 40:
       for (j=0;j<num_sects;j++)     /* remote door open */
         if (sectors[j].tag==linedefs[i].tag)
           raise_ceil(j,linedefs[i].type==40 ? 0 : 4);
       break;
     case 10: case 21: case 88: case 62: case 120:
     case 121: case 122: case 123: case 38: case 23:
     case 82: case 60: case 37: case 84: case 9:
       for (j=0;j<num_sects;j++)     /* lift or floor lower */
         if (sectors[j].tag==linedefs[i].tag)
           lower_floor(j);
       break;
     case 19: case 102: case 83: case 45:
       for (j=0;j<num_sects;j++)     /* floor lower nearest */
         if (sectors[j].tag==linedefs[i].tag)
           lowern_floor(j,0);
       break;
     case 36: case 71: case 98: case 70:
       for (j=0;j<num_sects;j++)     /* floor lower turbo */
         if (sectors[j].tag==linedefs[i].tag)
           lowern_floor(j,8);
       break;
     case 8: case 7: case 100: case 127:
       for (j=0;j<num_sects;j++)     /* stairs */
         if (sectors[j].tag==linedefs[i].tag)
           build_stairs(j,linedefs[i].type>=100 ? 16 : 8,sector_limits[j].floorh.min);
       break;

     case 3: case 50: case 75: case 42: case 16: case 76:
     case 110: case 113: case 107: case 116: case 41:
     case 43: case 44: case 49: case 72: case 6: case 25:
     case 73: case 77: case 141:
       for (j=0;j<num_sects;j++)     /* ceiling-lowering effects */
         if (sectors[j].tag==linedefs[i].tag)
           sector_limits[j].ceilh.moves=1;
       break;
     case 119: case 128: case 18: case 69: case 22:
     case 95: case 20: case 68: case 47: case 5:
     case 91: case 101: case 64: case 24: case 130:
     case 131: case 129: case 132: case 56: case 94:
     case 55: case 65: case 58: case 92: case 15:
     case 66: case 59: case 93: case 14: case 67:
     case 140: case 30: case 96: case 53: case 87:
       for (j=0;j<num_sects;j++)      /* floor-raising effects */
         if (sectors[j].tag==linedefs[i].tag)
           sector_limits[j].floorh.moves=1;
       break;
    }
}

static void mark_visplane(const struct view *v)
{
 int i=v->start_markers;
 struct Thing *tp=things+i;
 for (;i<num_things;i++,tp++)
  {
   long dx=v->x-tp->xpos;
   long dy=v->y-tp->ypos;
   if (dx<0) dx=-dx;
   if (dy<0) dy=-dy;
   if (dx<=64 && dy<=64) return;
  }
 things = ResizeMemory(things,(num_things+1)*sizeof(*things));
 tp=things + num_things++;
 tp->xpos=v->x;
 tp->ypos=v->y;
 tp->angle=(unsigned)(v->angle*(1.0/8192.0))*45;
 tp->type=1;  /* player 1 start (voodoo doll) */
 tp->when=7;  /* all levels */
}

#define VPOF_MAX_SECTOR 2  /* Maximum number of visplane warnings per sector */

void warn_visplanes(const struct Node *tn)
{
 static struct view v;
 int i;
 unsigned char *sector_vp_hitlist=NULL;

 puts("\nLooking for visplane overflows\n");

 sector_limits=GetMemory(num_sects*sizeof(*sector_limits));

 find_sector_limits();

 init_view(&v);

 for (i=0;i<num_psegs;i++)
  {
   int l = psegs[i].linedef;
   int vp;

   progress();           	        /* Something for the user to look at.*/

   vp = visplanes(tn,psegs+i, &v);

   if (vp>threshold)
    {
     int sector=sidedefs[psegs[i].flip ? linedefs[l].sidedef2 :
                                         linedefs[l].sidedef1 ].sector;
     if (mark_visplanes)
       mark_visplane(&v);
     if (!sector_vp_hitlist)
       memset( (sector_vp_hitlist = GetMemory(num_sects)), 0, num_sects);

     if (sector_vp_hitlist[sector] <= VPOF_MAX_SECTOR)
      {
       if (sector_vp_hitlist[sector]++ < VPOF_MAX_SECTOR)
        {
         printf(
                "Warning: VPOF (%d) near linedef %4d, sector %d, (%ld,%ld) angle %d\n",
                 vp, l, sector, v.x, v.y, (int)(v.angle*(360.0/65536.0)+.5)
               );
        }
       else
         printf("Further visplane overflow warnings suppressed in sector %d\n",sector);
      }
    }
  }
 printf(" \nMaximum number of visplanes=%d, near linedef %d, sector %d\n",
   v.max_visplanes, v.max_visplanes_linedef, v.max_visplanes_sector);
 free(sector_vp_hitlist);
 free(sector_limits);
 sector_limits=NULL;
 free_view(&v);                /* Free the view structures */
}
