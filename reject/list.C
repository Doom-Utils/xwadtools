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

// *****************************************************************************
//
//  MODULE: LIST - Implemetation of a Doubly linked list
//
// *****************************************************************************

#include "list.hpp"

// *****************************************************************************
//
//  FUNCTION: Add an CLink Object to the start of the linked list.
//
//  ENTRY: Pointer to a Clink
//
// *****************************************************************************

void CList::AddItem (CLink *link)
{
     link->next = base;
     if (base)
          base->prev = link;
     base = link;
}
   
// *****************************************************************************
//
//  FUNCTION: Remove an item from the list and delete it.
//
//  ENTRY:   link = pointer to the link to delete.
//
// *****************************************************************************

void CList::DeleteItem (CLink *link)
{
     CLink *prev;
     CLink *next;
     
     if (link)
     {
          prev = link->prev;          
          next = link->next;
     
          delete link;
     
          if (prev) 
               prev->next = next;
          else
               base = next;
               
          if (next) next->prev = prev;
     }
}                


// *****************************************************************************
//
//  FUNCTION: Walks the list of CLinks and deletes each link.
//
// *****************************************************************************

void CList::DeleteAll ()
{
     CLink *temp;

     while (base)
     {
          temp   = base->next;        
          delete base;
          base  = temp;
     }
}

// *****************************************************************************
//
//  FUNCTION:  Clist Destructor. Walks the list of CLinks and deletes each link.
//
// *****************************************************************************

CList::~CList ()
{
     DeleteAll ();
}
