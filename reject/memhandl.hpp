#ifndef __INC_MEMHANDL_HPP__  
#define __INC_MEMHANDL_HPP__  

#include <stdlib.h>
#include <string.h>

// *****************************************************************************
//
//  CLASS: MemoryBlock.
//
//     This is a private class internally by the MemHandle class. It is this
//     class which has custody of a block of memory. The constructor allocates
//     a block of memory of the specified size. When this object is destroyed
//     block of memory it is responsible for is also deleted.
//
// *****************************************************************************

class MemoryBlock
{
     friend class MemHandle;                 // only MemHandle objects can use this class
     private:
          MemoryBlock (size_t size);         // CTOR
          ~MemoryBlock ();                   // DTOR
          size_t szbuffer;                   // the size of the block of memory.
          int count;                         // the number of times this block
                                             // is referenced by MemHandle
					     // Objects
          unsigned char *buffer;             // pointer to the block of memory. 
};


// *****************************************************************************
//
//  CLASS: MemHandle
//
//
//
//
//
//
//
// *****************************************************************************

class MemHandle
{
     private:
          MemoryBlock *mem; 

     public:
          MemHandle (const MemHandle &mh);
          MemHandle (size_t size);
          MemHandle (size_t size, unsigned char filler);
          ~MemHandle ();

          const char* Buffer() const;
          MemHandle Clone ()  const;
          size_t Size ()      const;
          int Valid ()        const;
          int RefCount ()     const;
          void MemSet (int c);
          operator char * ();
          MemHandle& operator= (const MemHandle& m);
          void Shrink (size_t);
};


// *****************************************************************************
//        INLINE FUNCTIONS
// *****************************************************************************

inline const char* MemHandle::Buffer() const
{
     return (char*)(mem->buffer);
}


// *****************************************************************************
//
//  FUNCTION: MemHandle::Size () const  
//
//  RETURNS:  the size (in bytes) of the memory block held by the MemHandle
//	      object
//
// *****************************************************************************

inline size_t MemHandle::Size () const
{
     return (mem->szbuffer);
}


// *****************************************************************************
//
//  FUNCTION: MemHandle::Size () const  
//
//  RETURNS:  the size (in bytes) of the memory block held by the MemHandle
//	      object
//
// *****************************************************************************

inline int MemHandle::RefCount () const
{
     return (mem->count);
}


// *****************************************************************************
//
//  FUNCTION: MemHandle::Valid () const
//
//            Tests the validity of a MemHandle object
//            You _should_ test any MemHandle object before 
//            you use it. (except for anonomous temporary objects 
//            which you cant test. ho hum)
//
//  RETURNS: TRUE if the object is valid else FALSE
//
// *****************************************************************************

inline int MemHandle::Valid () const
{
     return mem && (mem->buffer);
}


// *****************************************************************************
//
//  FUNCTIONS: operator MemHandle::char * () 
//
//            Conversion operator to convert a MemHandle object to a pointer
//
//  RETURNS:  char * to the memory block held by the MemHandle object
//
// *****************************************************************************

inline MemHandle::operator char * ()
{
     return (char*)(mem->buffer);
}


// *****************************************************************************
//
//  FUNCTION: MemHandle::MemSet (int c)   
//
//   sets the whole buffer to the value c.
//
// *****************************************************************************

inline void MemHandle::MemSet (int c)
{
     memset (mem->buffer, c, mem->szbuffer);
}


#endif
