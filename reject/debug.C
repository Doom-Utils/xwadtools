#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <time.h>

#include "debug.hpp"

cdebugclass::cdebugclass ()
{
     char *debug_file;

     debug_file = getenv ("DEBUG_DEVICE");
     if (debug_file)
     {
          out.open (debug_file);

          if (out.good())
          {
               out << "Program Started at " << __TIME__ << " " << __DATE__ \
		   << "\n\n" << endl;
          }
     }
}

cdebugclass::~cdebugclass ()
{
     if (out.good())
     {
          out << "\nProgram Terminated at " << __TIME__ << " " << __DATE__ \
	      << "\n\n";

          out.close();
     }
}

cdebugclass cdebug;
