#ifndef __PWAD_HPP__                
#define __PWAD_HPP__                

#include "structs.hpp"
#include "ilist.hpp"
#include "fileio.hpp"
#include "dir.hpp"

class CPWad : public CBase
{
     private:
          CFile file;
          CDirEntry *currentmap;

          PWAD_HEADER header;
          long size;
          CIndexedList directory;

          void AddEntry (DIR_ENTRY &entry);

     public:
          CPWad (XString name);     
          MemHandle Read (XString map, XString item);
          MemHandle Read (XString item);

          int Write (MemHandle data, XString map, XString item);
          int Write (MemHandle data, XString item);

};
                                  
#endif                            
