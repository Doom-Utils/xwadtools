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

#include "ilist.hpp"
#include "xstring.hpp"

// *****************************************************************************
//
//  AUTHOR: L.M.Witek   DATE:26-Jan-1994
//
//  FUNCTION: Add an CIndexedLink Object to the linked list.
//
//  ENTRY: Pointer to a CIndexedLink Object
//
// *****************************************************************************

void CIndexedList::AddItem (CIndexedLink *link)
{    
     //  -=- if list is empty then place in 1st position in list -=-
     if (base == 0)
     {
          base = link;
     }
     else
     {    
          // -=- get pointer to start of list -=-       
          CIndexedLink *cursor = FirstItem();
          
          // -=- find position in list to insert object -=-
          while (cursor)
          {
               if (*cursor > *link)
               {
                    // -=- is new link to be first in list? -=-
                    if (cursor->prev == 0)
                    {
                         base = link;               
                    }
                    else 
                    {    
                         ((CIndexedLink*)(cursor->prev))->next = link; 
                         link->prev = cursor->prev;
                    }
                    // -=- add item into list -=-
                    link->next = cursor;
                    cursor->prev = link; 

                    break;
               }
               
               // -=- if at end of list add to end of list -=-
               if (cursor->next == 0)
               {
                    cursor->next = link;
                    link->prev = cursor;
                    break;
               }
               cursor = ((CIndexedLink*)(cursor->next));
          }
     }
}                


CIndexedLink *CIndexedList::operator[] (XString key)
{
     CIndexedLink *cursor = FirstItem();
     while (cursor)
     {                   
          if (key == cursor->key)
               break;
          cursor = cursor->Next();
     }     
     return cursor;
}
