/*- MAKENODE.C --------------------------------------------------------------*
 Recursively create nodes and return the pointers.
*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
 Split a list of segs (ts) into two using the method described at bottom of
 file, this was taken from OBJECTS.C in the DEU5beta source.

 This is done by scanning all of the segs and finding the one that does
 the least splitting and has the least difference in numbers of segs on either
 side.
 If the ones on the left side make a SSector, then create another SSector
 else put the segs into lefts list.
 If the ones on the right side make a SSector, then create another SSector
 else put the segs into rights list.
*---------------------------------------------------------------------------*/

static int DivideSegs(struct Seg *ts,struct Seg *best,struct Seg **rs,struct Seg **ls)
{
	struct Seg *rights=NULL,*lefts=NULL;
	struct Seg *tmps,*news,*prev;
	struct Seg *add_to_rs,*add_to_ls;
  	struct Seg *new_best=NULL,*new_rs,*new_ls;
	struct Seg *strights=NULL,*stlefts=NULL;

        *rs=*ls=NULL;

/*
        printf("Picked linedef %d (%d,%d) (%d) as node line\n",
        best->linedef, best->start, best->end, best->flip);
*/

/* When we get to here, best is a pointer to the partition seg.
   Using this partition line, we must split any lines that are intersected
   into a left and right half, flagging them to be put their respective sides
   Ok, now we have the best line to use as a partitioning line, we must
   split all of the segs into two lists (rightside & leftside).				 */

	for(tmps=ts;tmps;tmps=tmps->next)
         {
          long a = best->pdy * tmps->psx - best->pdx * tmps->psy + best->ptmp;
          long b = best->pdy * tmps->pex - best->pdx * tmps->pey + best->ptmp;

          if ((a^b) < 0)
            if (a && b)
	     {                    /* Seg is split */
                double ds = (double) a / (a-b);  /* 0 = start, 1 = end */
                int x = tmps->psx + tmps->pdx * ds + 0.5;
	        int y = tmps->psy + tmps->pdy * ds + 0.5;

                if (x==tmps->psx && y==tmps->psy)
		 {
                  if (b<0)
                    goto leftside;
                  else
                    goto rightside;
		 }

                if (x==tmps->pex && y==tmps->pey)
		 {
                  if (a<0)
                    goto leftside;
                  else
                    goto rightside;
		 }

/*		printf("Splitting Linedef %d at %d,%d\n",tmps->linedef,x,y); */

 	        vertices = ResizeMemory(vertices, sizeof(struct Vertex) * (num_verts+1));

		vertices[num_verts].x = x;
		vertices[num_verts].y = y;
  		news = GetMemory(sizeof( struct Seg));

                *news = *tmps;
		tmps->next = news;
		news->start = num_verts;
		tmps->end = num_verts;

                news->pdx = (long) (news->pex = vertices[news->end].x)
                                 - (news->psx = x);
                news->pdy = (long) (news->pey = vertices[news->end].y)
                                 - (news->psy = y);
                news->ptmp = news->pdx*news->psy - news->psx*news->pdy;
                news->len = (long) sqrt((double) news->pdx*news->pdx +
                                        (double) news->pdy*news->pdy);

                tmps->pdx = (long) (tmps->pex = x)
                                 - (tmps->psx = vertices[tmps->start].x);
                tmps->pdy = (long) (tmps->pey = y)
                                 - (tmps->psy = vertices[tmps->start].y);
                tmps->ptmp = tmps->pdx*tmps->psy - tmps->psx*tmps->pdy;
                tmps->len = (long) sqrt((double) tmps->pdx*tmps->pdx +
                                        (double) tmps->pdy*tmps->pdy);

                {
                 long dx,dy;
                 if (news->flip)
                  {
                   dx = vertices[linedefs[news->linedef].end].x;
                   dy = vertices[linedefs[news->linedef].end].y;
                  }
                 else
                  {
                   dx = vertices[linedefs[news->linedef].start].x;
                   dy = vertices[linedefs[news->linedef].start].y;
                  }
                 dx -= news->psx;
                 dy -= news->psy;
                 news->dist = (int)(sqrt((double) dx*dx + (double) dy*dy));
                }

/*		printf("splitting dist = %d\n",news->dist);*/
/*		printf("splitting vertices = %d,%d,%d,%d\n",tmps->start,tmps->end,news->start,news->end);*/

                if (a<0)
                 {
                  add_to_rs=news;
                  add_to_ls=tmps;
                 }
                else
                 {
                  add_to_ls=news;
                  add_to_rs=tmps;
                 }
		tmps = news;
		num_verts++;
             }
            else
             goto leftside;
          else
            if (a<=0 && (a || (!b && tmps->pdx*best->pdx+tmps->pdy*best->pdy < 0 )))
             {
              leftside:
              add_to_ls=tmps;     /* Seg is on left side */
              add_to_rs=NULL;
             }
            else
             {
              rightside:
              add_to_rs=tmps;     /* Seg is on right side */
              add_to_ls=NULL;
             }

	if (add_to_rs)	  /* CHECK IF SHOULD ADD RIGHT ONE */
         {
/*
          printf("Adding linedef %d (%d,%d) (%d) to right side\n",
          add_to_rs->linedef, add_to_rs->start, add_to_rs->end,
          add_to_rs->flip);
*/
	  new_rs = GetMemory(sizeof(struct Seg));
	  if (add_to_rs == best)
            new_best = new_rs;
          *new_rs = *add_to_rs;
	  new_rs->next = NULL;
 	  if (!rights)
            strights = rights = new_rs;
	  else
	   {
	    rights->next = new_rs;
	    rights = new_rs;
           }
	}

      if (add_to_ls)	/* CHECK IF SHOULD ADD LEFT ONE */
       {
/*
        printf("Adding linedef %d (%d,%d) (%d) to left side\n",
        add_to_ls->linedef, add_to_ls->start, add_to_ls->end,
        add_to_ls->flip);
*/
	new_ls = GetMemory(sizeof(struct Seg));
	if (add_to_ls == best)
          new_best = new_ls;
        *new_ls = *add_to_ls;
	new_ls->next = NULL;
	if (!lefts)
          stlefts = lefts = new_ls;
	else
         {
	  lefts->next = new_ls;
	  lefts = new_ls;
       	 }
      }
   }

	if(strights == NULL)
		{
/*		printf("No right side, moving partition into right side\n"); */
		strights = rights = new_best;
		prev = NULL;
		for(tmps=stlefts;tmps;tmps=tmps->next)
			{
			if(tmps == new_best)
				{
				if(prev != NULL) prev->next=tmps->next;
				else stlefts=tmps->next;
				}
			prev=tmps;
			}
		prev->next = NULL;
		}

	if(stlefts == NULL)
		{
/*		printf("No left side, moving partition into left side\n"); */
		stlefts = lefts = new_best;
		prev = NULL;
		for(tmps=strights;tmps;tmps=tmps->next)
			{
			if(tmps == new_best)
				{
				if(prev != NULL) prev->next=tmps->next;
				else strights=tmps->next;
				}
			prev=tmps;
			}
		stlefts->next = NULL;
		prev->next = NULL;        /* Make sure end of list = NULL*/
		}

        rights->next = NULL;
	lefts->next = NULL;

	*rs = strights;
        *ls = stlefts;
        return 0;
}

/*--------------------------------------------------------------------------*/

static int IsItConvex(const struct Seg *ts)
{
   const struct Seg *line;
   register const struct Seg *check;
   int sector;

   for (line=ts; line; line=line->next)
     if (linedefs[line->linedef].tag >= 900 &&
         linedefs[line->linedef].flags & 4 &&
         sidedefs[linedefs[line->linedef].sidedef1].sector==
         sidedefs[linedefs[line->linedef].sidedef2].sector)
       for (check=line->next;check;check=check->next)
         if (check->linedef==line->linedef && check->flip!=line->flip)
           return FALSE;

  /* All ssectors must come from same sector unless it's marked
     "special" with sector tag >= 900. Original idea, Lee Killough */

   for (check=ts; check; check=check->next)
     if (linedefs[check->linedef].tag < 900 &&
         sectors[sector=check->sector].tag < 900)
      {
       while ((check=check->next)!=NULL)
         if (check->sector != sector &&
             sectors[check->sector].tag < 900 &&
             linedefs[check->linedef].tag < 900 )
           return FALSE;
       break;
      }

   /* all of the segs must be < 180 degrees apart */

   for (line=ts;line;line=line->next)
     for (check=ts;check;check=check->next) /* Check partition against all Segs*/
      { /*     get state of lines' relation to each other    */
       long a = line->pdy * check->psx - line->pdx * check->psy + line->ptmp;
       long b = line->pdy * check->pex - line->pdx * check->pey + line->ptmp;

       if ((a^b)>=0 ? a<0 || b<0 || (!a && !b && check->pdx*line->pdx +
			      check->pdy*line->pdy < 0 &&
		      (check->linedef != line->linedef ||
		       !(linedefs[line->linedef].flags & 4) || 
		       linedefs[line->linedef].sidedef1==-1 || 
		       *sidedefs[linedefs[line->linedef].sidedef1].tex3-'-' ||
		       linedefs[line->linedef].sidedef2==-1 || 
		       *sidedefs[linedefs[line->linedef].sidedef2].tex3-'-'
		       )
		      )
	   : (b<0 || check->len*a/(a-b)>=2) && (a<0 || check->len*b/(b-a)>=2))
         return FALSE;
     }
  /* no need to split the list: these Segs can be put in a SSector */
   return TRUE;
}

/*--------------------------------------------------------------------------*/

static int CreateSSector(struct Seg *tmps)
{
  int n=0;
  struct Seg *ts;

  for (ts=tmps;ts;ts=ts->next)
    n++;

  ssectors = num_ssectors ? ResizeMemory(ssectors,sizeof(struct SSector)*(num_ssectors+1))
                          : GetMemory(sizeof(struct SSector));

  ssectors[num_ssectors].num = n;

  ssectors[num_ssectors].first = num_psegs;

  psegs = num_psegs ? ResizeMemory(psegs,sizeof(struct Pseg)*(num_psegs+n))
                    : GetMemory(sizeof(struct Pseg)*n);

  for (;tmps;tmps=tmps->next)
   {
    psegs[num_psegs].start = tmps->start;
    psegs[num_psegs].end = tmps->end;
    psegs[num_psegs].angle = tmps->angle;
    psegs[num_psegs].linedef = tmps->linedef;
    psegs[num_psegs].flip = tmps->flip;
    psegs[num_psegs++].dist = tmps->dist;
   }

  return num_ssectors++;
}

/*- translate (dx, dy) into an integer angle value (0-65535) ---------------*/

static unsigned ComputeAngle(long dx, long dy)
{
  double w = atan2( (double) dy , (double) dx ) * (32768.0/M_PI);
  if (w<0) w+=65536.0;
  return (unsigned) w;
}

static void DelSegs(struct Seg *ts)
{
 while (ts)
  {
   struct Seg *t=ts->next;
   free(ts);
   ts=t;
  }
}

static unsigned height(const struct Node *tn)
{
 if (tn)
  {
   unsigned l=height(tn->nextl),r=height(tn->nextr);
   return l>r ? l+1 : r+1;
  }
 return 1;
}

static struct Node *CreateNode(struct Seg *ts)
{
 struct Node node, *tn=&node;
 struct Seg *rights,*lefts,*best;

 best = PickNode(ts);           /* Pick best node to use.*/

 if (!best)                     /* Failsafe measure */
   best=ts;

 DivideSegs(ts,best,&rights,&lefts);

 if (!rights || !lefts)         /* Failsafe measure */
   return NULL;

 FindLimits(lefts);			/* Find limits of vertices   */
 tn->maxy2 = lmaxy;
 tn->miny2 = lminy;
 tn->minx2 = lminx;
 tn->maxx2 = lmaxx;

 tn->nextl = IsItConvex(lefts) ? NULL : CreateNode(lefts);

 FindLimits(rights);			/* Find limits of vertices*/
 tn->maxy1 = lmaxy;
 tn->miny1 = lminy;
 tn->minx1 = lminx;
 tn->maxx1 = lmaxx;

 tn->nextr = IsItConvex(rights) ? NULL : CreateNode(rights);

 if (!tn->nextl)
   tn->chleft = CreateSSector(lefts);

 if (!tn->nextr)
   tn->chright = CreateSSector(rights);

 tn->x  = best->psx;
 tn->y  = best->psy;
 tn->dx = best->pdx;
 tn->dy = best->pdy;

 tn->ptmp=tn->dx*tn->y-tn->dy*tn->x;

 DelSegs(ts);

 num_nodes++;
 tn = GetMemory(sizeof(*tn));	/* Create a node*/
 *tn = node;
 return tn;
}

/*---------------------------------------------------------------------------*

	This message has been taken, complete, from OBJECTS.C in DEU5beta source.
	It outlines the method used here to pick the nodelines.

	IF YOU ARE WRITING A DOOM EDITOR, PLEASE READ THIS:

   I spent a lot of time writing the Nodes builder.  There are some bugs in
   it, but most of the code is OK.  If you steal any ideas from this program,
   put a prominent message in your own editor to make it CLEAR that some
   original ideas were taken from DEU.  Thanks.

   While everyone was talking about LineDefs, I had the idea of taking only
   the Segs into account, and creating the Segs directly from the SideDefs.
   Also, dividing the list of Segs in two after each call to CreateNodes makes
   the algorithm faster.  I use several other tricks, such as looking at the
   two ends of a Seg to see on which side of the nodeline it lies or if it
   should be split in two.  I took me a lot of time and efforts to do this.

   I give this algorithm to whoever wants to use it, but with this condition:
   if your program uses some of the ideas from DEU or the whole algorithm, you
   MUST tell it to the user.  And if you post a message with all or parts of
   this algorithm in it, please post this notice also.  I don't want to speak
   legalese; I hope that you understand me...  I kindly give the sources of my
   program to you: please be kind with me...

   If you need more information about this, here is my E-mail address:
   Raphael.Quinet@eed.ericsson.se (Rapha‰l Quinet).

   Short description of the algorithm:
     1 - Create one Seg for each SideDef: pick each LineDef in turn.  If it
	 has a "first" SideDef, then create a normal Seg.  If it has a
	 "second" SideDef, then create a flipped Seg.
     2 - Call CreateNodes with the current list of Segs.  The list of Segs is
	 the only argument to CreateNodes.
     3 - Save the Nodes, Segs and SSectors to disk.  Start with the leaves of
	 the Nodes tree and continue up to the root (last Node).

   CreateNodes does the following:
     1 - Pick a nodeline amongst the Segs (minimize the number of splits and
	 keep the tree as balanced as possible).
     2 - Move all Segs on the right of the nodeline in a list (segs1) and do
	 the same for all Segs on the left of the nodeline (in segs2).
     3 - If the first list (segs1) contains references to more than one
	 Sector or if the angle between two adjacent Segs is greater than
	 180ø, then call CreateNodes with this (smaller) list.  Else, create
	 a SubSector with all these Segs.
     4 - Do the same for the second list (segs2).
     5 - Return the new node (its two children are already OK).

   Each time CreateSSector is called, the Segs are put in a global list.
   When there is no more Seg in CreateNodes' list, then they are all in the
   global list and ready to be saved to disk.

*---------------------------------------------------------------------------*/
