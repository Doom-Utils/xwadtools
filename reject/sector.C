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

#include "sector.hpp"

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))


CSector::CSector ()
        : max_x (-32768), max_y (-32768), min_x (32767), min_y (32767)
{
}

void CSector::AddVertex (VERTEX &v)
{
     max_x = max (max_x, v.x);
     max_y = max (max_y, v.y);
     min_x = min (min_x, v.x);
     min_y = min (min_y, v.y);
}

void CSector::GetCenter (SWORD &x, SWORD &y)
{
     x = max_x/2 + min_x/2;
     y = max_y/2 + min_y/2;
}
