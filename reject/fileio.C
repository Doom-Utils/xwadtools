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

#include "fileio.hpp"

BOOLEAN CFile::SetError (int exp)
{
     if (exp)
     {
          errorcode = errno;
          return (bFALSE);
     }
     else
     {
          errorcode = 0;
          return (bTRUE);
     }
}

void CFile::Close ()
{
     if (fh != -1)
          close (fh);     
}

BOOLEAN CFile::Open (XString name, int oflag)
{
     fh = open (name, oflag, S_IWRITE | S_IREAD);
     return SetError (fh == -1);
}

long CFile::Tell ()
{
     long pos = tell (fh);
     SetError (pos == -1);
     return pos;
}

long CFile::Size ()
{
     long sz = filelength (fh);
     SetError (sz == -1);
     return sz;
}

long CFile::Seek (long pos)
{
     long newpos = lseek (fh, pos, SEEK_SET);
     SetError (newpos == -1);
     return newpos;
}

BOOLEAN CFile::IsOpen ()
{
     if (fh == -1)
          return (bFALSE);
     else
          return (bTRUE);
}

int CFile::Read (void *buffer, size_t size)
{
     int r = read (fh, buffer, size);
     SetError (r == -1);
     return r;
}

MemHandle CFile::Read (size_t size)
{
     MemHandle buffer (size);
     int r = read (fh, buffer, size);
     SetError (r == -1);
     if ((size_t) r < size)
          buffer.Shrink (r);
     return buffer;
}

int CFile::Write (MemHandle buffer)
{
     int written = write (fh, buffer, buffer.Size ());
     SetError (written == -1);
     return written;
}
