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

#include <string.h>
#include "memhandl.hpp"

// *****************************************************************************
// *****************************************************************************
// *
// *      Implementation of the MemoryBlock class
// *
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
//
//  CONSTRUCTOR: MemoryBlock::MemoryBlock (size_t size)  
//
//               Allocate buffer & set ref count to 0.
//
// *****************************************************************************

MemoryBlock::MemoryBlock (size_t size) 
            : szbuffer (size), count (0), buffer (new unsigned char[size]) 
{
}


// *****************************************************************************
//
//  CONSTRUCTOR: MemoryBlock::~MemoryBlock ()  
//
//               Delete the block of memory.
//
// *****************************************************************************

MemoryBlock::~MemoryBlock () 
{ 
     delete [] buffer;
}


// *****************************************************************************
// *****************************************************************************
// *
// *      Implementation of the MemHandle class
// *
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
//
//  CONSTRUCTOR: MemHandle::MemHandle (MemHandle &mh)  
//
//              Constructs a memory handle to an existing block of memory 
//              specified by the 'mh' parameter.
//
// *****************************************************************************

MemHandle::MemHandle (const MemHandle &mh)
{
     mem = mh.mem; 
     (mem->count)++;
}


// *****************************************************************************
//
//  CONSTRUCTOR: MemHandle::MemHandle (size_t size)  
//
//              Constructs a memory handle to a new block of memory of 'size' 
//              bytes. The block of memory is not initialised to any specific 
//              value.
//
// *****************************************************************************

MemHandle::MemHandle (size_t size)
          :mem (new MemoryBlock (size))
{    
     (mem->count)++;
}


// *****************************************************************************
//
//  CONSTRUCTOR: MemHandle::MemHandle (size_t size, unsigned char filler)  
//
//              Constructs a memory handle to a new block of memory of 'size' 
//              bytes and initialises the block to the value specified by
//              the 'filler' parameter.
//
// *****************************************************************************

MemHandle::MemHandle (size_t size, unsigned char filler)
          :mem (new MemoryBlock (size))
{
     (mem->count)++;
     memset (mem->buffer, filler, size);
}


// *****************************************************************************
//
//  DESTRUCTOR: MemHandle::~MemHandle 
//
//              Destroys the memory handle. If the block of memory associated  
//              with this handle has no other handles referencing it then it 
//              id deleted also.
//
// *****************************************************************************

MemHandle::~MemHandle ()
{
     (mem->count)--;
     if (mem->count == 0) delete mem;
}


// *****************************************************************************
//
//  FUNCTION: MemHandle::Clone () const  
//
//            Make a copy of the memory block held by the MemHandle object.
//
//  RETURNS:  a MemHandle to the cloned memory block 
//
// *****************************************************************************

MemHandle MemHandle::Clone () const
{
     MemHandle mh (Size());	// make a memory block the same size as me
				// copy my data to this new memory block
     memcpy (mh.mem->buffer, mem->buffer, Size());

     return (mh);		// return the handle to this new memory block
}


// *****************************************************************************
//
//  FUNCTION: MemHandle& MemHandle::operator= (const MemHandle& mh)
//
//            Assign a new memory block to this handle
//
// *****************************************************************************

MemHandle& MemHandle::operator= (const MemHandle& mh)
{
     if (mh.mem != mem)		// make sure we dont copy ourself
     {
          (mem->count)--;	// detatch from old memory block 
          if (mem->count == 0) delete mem;
               
          mem = mh.mem;		// attach to new memory block
          (mem->count)++;
     }
     return *this;
}


// *****************************************************************************
//
//  FUNCTION: void MemHandle::Shrink (size_t newsz)
//
//            Shrink the size of a memory block. Does not change the memory 
//            the memory allocation just marks the block a being smaller.
//            Useful for creating a buffer to hold some data where the data 
//            size is unknown. Memory block can be shrunk after data is  
//            placed in buffer.
//
// *****************************************************************************

void MemHandle::Shrink (size_t newsz)
{
     if (newsz < mem->szbuffer)
          mem->szbuffer = newsz;
}


// *****************************************************************************
// *****************************************************************************
// *
// *      Test rig for the MemHandle class
// *
// *****************************************************************************
// *****************************************************************************
//#define _TEST_MEMHANDLE_

#ifdef _TEST_MEMHANDLE_

#include <stdio.h>

void main ()
{
     MemHandle mh1 (1024);
     MemHandle mh2 (512);
     MemHandle mh3 (mh2);

     strcpy (mh1, "First Block");
     strcpy (mh2, "Second Block");
     
//     memcpy (mh1, mh2, 3);

     {    
          MemHandle mhx (mh3);
     }


     puts (mh1);
     puts (mh2);

     mh1 = mh2;
     mh1 = mh2;

     puts (mh1);

     puts (mh1);  

}

#endif
