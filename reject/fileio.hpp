#ifndef __FILEIO_HPP__                
#define __FILEIO_HPP__                

#ifdef UNIX
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifdef MSDOS
#include <io.h>
#include <errno.h>
#include <dos.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys\stat.h>
#endif

#include "constant.hpp"
#include "xstring.hpp"

#include "sysdep.h"

class CFile
{
     private:
          int fh;
          int errorcode;
          BOOLEAN SetError (int exp);

     public:
          CFile (XString name, int oflag);
          CFile ();
          ~CFile ();

          BOOLEAN Open (XString name, int oflag);
          void Close ();

          long    Seek (long pos);
          long    Tell  ();
          long    Size  ();
          BOOLEAN IsOpen ();

          MemHandle Read (size_t size);          
          int Read (void *buffer, size_t size);
          int Write (MemHandle buffer);
          int Error ();
};


inline CFile::CFile (XString name, int oflag) : errorcode (0)
{ 
     Open (name, oflag); 
}

inline CFile::CFile () : fh (-1), errorcode (0) 
{
}

inline CFile::~CFile () 
{ 
     Close (); 
}

inline int CFile::Error ()
{
     return errorcode;
}


#endif                            
