#ifndef __DEBUG_HPP__                
#define __DEBUG_HPP__                

#include <fstream.h>

class cdebugclass
{
     public:
          ofstream out;
          cdebugclass ();
          ~cdebugclass ();
};

extern cdebugclass cdebug;

#ifdef DEBUG

#define ONDEBUG(x) x

#else

#define ONDEBUG(x) 

#endif

#endif                            
