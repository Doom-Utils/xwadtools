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
#include <new.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream.h>

#include "pwad.hpp"
#include "reject.hpp"

extern char* version_info;

void NoMemory (void)
{
     cerr << "\n Out Of Memory" << endl;
     exit(1);
}

int main (int argc, char** argv)
{
     set_new_handler (NoMemory);

     cerr << version_info << endl;
     if (argc > 3)
     {
          // -=- Read in PWAD directory -=- 
          //CPWad wad (XString(argv[1]) + XString(".WAD"));
	  CPWad wad = XString(argv[1]);

          // -=- All OK ? -=-
          if (!wad.Valid ())
          {
               cerr << "Failed Reading PWAD " << argv[1] << endl;
               exit (0);
          }

          XString map (argv[2]);
          int threshold = atoi (argv[3]);

          if ((threshold >= 32) && (threshold <= 4096))
          {
               CReject rej (wad, map);
               if (rej.Valid ())
               {
                    rej.RejectAll ();
                    cout << "Calculating Reject table...." << flush;
                    rej.RejectDefault (threshold);
                    cout << "Done." << endl;
          
                    cout << "Writing Reject table...." << flush;
                    wad.Write (rej.GetRejectData(), map, "REJECT");
                    cout << "Done." << endl;
               }
          }
          else
          {
               cerr << "Treshold must be between 32 and 4096" << endl;
          }
     }
     else
     {
          cerr << "USAGE: REJECT <WADNAME> <MAPNAME> <THRESHOLD>" << endl;
          cerr << "   eg: reject mywad e1m1 600" << endl;
     }
     return 0;
}
