// *****************************************************************************
//    REJECT.EXE - Reject data table builder for DOOM
//    Copyright (C) 1994 L.M.WITEK 
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 1, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
// *****************************************************************************

#include <iostream.h>
#include "debug.hpp"
#include "reject.hpp"
#include "swapfunc.h"

CReject::CReject (CPWad &w, XString map)
        : pwad (w),
          sect (0),
          rejectdata  (pwad.Read (map, "REJECT")),
          linedefdata (pwad.Read (map, "LINEDEFS")),
          sidedefdata (pwad.Read (map, "SIDEDEFS")),
          vertexdata  (pwad.Read (map, "VERTEXES")),
          sectordata  (pwad.Read (map, "SECTORS")),
          linedef ((LINEDEF *)linedefdata.Buffer()),
          sidedef ((SIDEDEF *)sidedefdata.Buffer()),
          vertex  ((VERTEX  *)vertexdata.Buffer()),
          sector  ((SECTOR  *)sectordata.Buffer())
{
     if (rejectdata.Size () && linedefdata.Size () && sidedefdata.Size () && vertexdata.Size () && sectordata.Size ())
     {
       swap_linedef_list(linedef, linedefdata.Size()/sizeof(LINEDEF));
       swap_sidedef_list(sidedef, sidedefdata.Size()/sizeof(SIDEDEF));
       swap_vertex_list(vertex, vertexdata.Size()/sizeof(VERTEX));
       swap_sector_list(sector, sectordata.Size()/sizeof(SECTOR));
          numsectors = sectordata.Size() / sizeof (SECTOR);
          sect = new CSector [numsectors];
          cdebug.out << "Number of sectors in " << map << ": " << numsectors << endl;

          int numlinedefs (linedefdata.Size() / sizeof (LINEDEF));

          for (int n = 0; n < numlinedefs; n++)
          {
               if (linedef[n].sidedef1 != 0xffff)
               {
                    sect[sidedef[linedef[n].sidedef1].sector].AddVertex (vertex[linedef[n].from_vertex]);
                    sect[sidedef[linedef[n].sidedef1].sector].AddVertex (vertex[linedef[n].to_vertex]);
               }

               if (linedef[n].sidedef2 != 0xffff)
               {
                    sect[sidedef[linedef[n].sidedef2].sector].AddVertex (vertex[linedef[n].from_vertex]);
                    sect[sidedef[linedef[n].sidedef2].sector].AddVertex (vertex[linedef[n].to_vertex]);
               }
          }

          for (int m = 0; m < numsectors; m++)
          {
               SWORD x, y;
               sect[m].GetCenter (x, y);
               cdebug.out << "Sector " << m << " X: " << x << " Y: " << y << endl;
          }
     }
     else
     {
          Fail ();
     }
}

// *****************************************************************************
//
//        A Fast Approximation to the Hypotenuse by Alan Paeth  
//        from "Graphics Gems Volume I", Academic Press, 1990
//        gives approximate distance from (x1,y1) to (x2,y2)
//        with only overestimations, and then never by more
//        than (9/8) + one bit uncertainty.
//
// *****************************************************************************

int CReject::idist (int x1, int y1, int x2, int y2)
{
     if ((x2 -= x1) < 0) x2 = -x2;
     if ((y2 -= y1) < 0) y2 = -y2;
     return (x2 + y2 - (((x2>y2) ? y2 : x2) >> 1));
}


void CReject::SetBit (char* buffer, int bit)
{
     buffer[bit/8] |= (1 << (bit ^ 8)); 
}

void CReject::ResetBit (char* buffer, int bit)
{
     buffer[bit/8] &= ~(0x1 << (bit % 8)); 
}

CReject::~CReject ()
{
     delete[] sect;
}

void CReject::RejectDefault (int threshold)
{
     SWORD x1, y1;
     SWORD x2, y2;

     RejectAll ();

     for (int n = 0; n < numsectors; n++)
     {
          sect[n].GetCenter (x1, y1);
          for (int m = 0; m < numsectors; m++)
          {
               sect[m].GetCenter (x2, y2);
               if (idist (x1, y1, x2, y2) <= threshold)
                    AcceptSector (n, m);
          }
     }

}

void CReject::RejectSector (int local, int remote)
{
     SetBit (rejectdata, (remote * numsectors) + local);
}

void CReject::AcceptSector (int local, int remote)
{
     ResetBit (rejectdata, (remote * numsectors) + local);
}

void CReject::RejectAll ()
{
     rejectdata.MemSet (0xff);
}

void CReject::AcceptAll ()
{
     rejectdata.MemSet (0x00);
}

MemHandle CReject::GetRejectData ()
{
     return rejectdata;
}
