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

#include <stdio.h>
#include <iostream.h>
#include <iomanip.h>
#include <string.h>
#include "sysdep.h"
#include "swapfunc.h"
#include "debug.hpp"
#include "pwad.hpp"

// *****************************************************************************
//
//  AUTHOR: L.M.Witek   DATE:26-May-1994
//
//  FUNCTION: CTOR for PWAD class. Opens the PWAD and builds an internal
//             dirctory for records in the PWAD.
//
//  ENTRY:   name of the pwad to open.
//
// *****************************************************************************

CPWad::CPWad (XString name)
      : file (name, O_RDWR | O_BINARY), currentmap(0)
{
     if (file.IsOpen())
     {
          // -=- read the pwad header -=-
          cout << "Opened '" << name << "'" << endl;
          cdebug.out << "Opened '" << name << "'" << endl;
          file.Read (&header, sizeof(PWAD_HEADER));
          swaplong((int *)&(header.dir_offset));
          swaplong((int *)&(header.dir_entries));
          if (memcmp ("PWAD", header.signature, 4) != 0)
          {
               cout << "Not a PWAD!" << "\n";
               cdebug.out << "Not a PWAD!" << "\n";
               Fail ();
               return;
          }
          else
          {
               cdebug.out << "File size: " << file.Size() << endl;
               cdebug.out << "Directory Offset: " << header.dir_offset << endl;
               cdebug.out << "Directory Entries: " << header.dir_entries << endl;
          }

          // -=- read the directory -=-
          file.Seek (header.dir_offset);
          MemHandle dirbuff = file.Read ((size_t)(header.dir_entries * sizeof(DIR_ENTRY)));
          if (dirbuff.Size () != (header.dir_entries * sizeof(DIR_ENTRY)))
          {
               cout << "Error Reading PWAD Directory." << endl;
               cdebug.out << "Error Reading PWAD Directory." << endl;
               Fail ();
          }
          else
          {
               cout << "Building directory...." << flush;
               swap_lump_list((void *)dirbuff.Buffer(), header.dir_entries);
               // -=- go through each dir entry and add it to our internal dir -=-
               DIR_ENTRY *entry_ptr = (DIR_ENTRY *)dirbuff.Buffer();
               
               for (int n = 0; n < header.dir_entries; n++)
               {
                    AddEntry (entry_ptr[n]);
               }
               cout << "Done." << endl;

// -=- print out directory used for debugging -=- 
               CDirEntry *de;
               CDirEntry *sde;
               de = (CDirEntry *)directory.FirstItem ();
               while (de)
               {
                    cdebug.out << de->GetKey() << "  " << de->Size() << " bytes\t  @" << de->Offset() << endl;
                    sde = (CDirEntry *)de->subdir.FirstItem ();
                    while (sde)
                    {
                         cdebug.out << "  " << sde->GetKey() << "  "  << sde->Size() << " bytes\t  @" << sde->Offset() << endl;
                         sde = (CDirEntry *)sde->Next();
                    }

                    de = (CDirEntry *)de->Next();
               }
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
          }
     }
     else
     {
          cout << "Cannot open '" << name << "'" << endl;
          cdebug.out << "Cannot open '" << name << "'" << endl;
          Fail ();
     }
}

void CPWad::AddEntry (DIR_ENTRY &entry)
{
     char namebuf[9];
     namebuf[8] = 0;
     memcpy (namebuf, entry.resource_name, 8);
     
     XString name (namebuf);

     // -=- test for map level marker -=- 
     if ((((name[0] == 'E') && (name[2] == 'M')) || ((name[0] == 'M') && (name[1] == 'A') && (name[2] == 'P'))) && (entry.resource_size == 0))
     {
          // -=- add map header entry to dir and set up current map -=-
          directory.AddItem (currentmap = new CDirEntry (name, 0, 0));
     }
     else
     {
          // -=- put all map related records in the list in the sub dir of the map -=-
          // -=- all other items go in the main dir -=-
          if  ((name == XString("THINGS")) ||
		(name == XString("LINEDEFS")) ||
		(name == XString("SIDEDEFS")) ||
		(name == XString("VERTEXES")) ||	
		(name == XString("SEGS")) ||
		(name == XString("SSECTORS")) ||
		(name == XString("NODES")) ||
		(name == XString("SECTORS")) ||	
		(name == XString("REJECT")) ||
		(name == XString("BLOCKMAP")))
          {
               currentmap->subdir.AddItem (new CDirEntry (name, entry.resource_offset, entry.resource_size)); 
          }
          else
          {
               directory.AddItem (new CDirEntry (name, entry.resource_offset, entry.resource_size)); 
               currentmap = 0;
          }
     }
}

MemHandle CPWad::Read (XString map, XString item)
{

     CDirEntry *entry = (CDirEntry *)directory[map];
     if (entry)
     {
          entry = (CDirEntry *)entry->subdir[item];
          if (entry)
          {
               file.Seek (entry->Offset());
               MemHandle data = file.Read ((size_t)entry->Size());
               if (data.Size() == (size_t) entry->Size())
               {
                    cdebug.out << "Read From PWAD " << map << " " << item << " " << data.Size() << " bytes" << endl;
                    return data;
               }
          }
     }
     cerr << "Read From PWAD " << map << " " << item << " FAILED!! " << endl;
     cdebug.out << "Read From PWAD " << map << " " << item << " FAILED!! " << endl;
     return 0;
}

int CPWad::Write (MemHandle data, XString map, XString item)
{

     CDirEntry *entry = (CDirEntry *)directory[map];
     if (entry)
     {
          entry = (CDirEntry *)entry->subdir[item];
          if (entry)
          {
               file.Seek (entry->Offset());
               if (data.Size() == (size_t) entry->Size())
               {
                    int status = file.Write (data);

                    cdebug.out << "Write to PWAD " << map << " " << item << " " << data.Size() << " bytes" << endl;
                    return status;
               }
          }
     }
     cerr << "Write to PWAD " << map << " " << item << " FAILED!! " << endl;
     cdebug.out << "Write to PWAD " << map << " " << item << " FAILED!! " << endl;
     return -1;
}


MemHandle CPWad::Read (XString item)
{
     CDirEntry *entry = (CDirEntry *)directory[item];
     if (entry)
     {
          file.Seek (entry->Offset());
          MemHandle data = file.Read ((size_t)entry->Size());
          if (data.Size() == (size_t) entry->Size())
               return data;
     }
     return 0;
}
