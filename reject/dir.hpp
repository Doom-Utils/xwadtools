#ifndef __DIR_HPP__                
#define __DIR_HPP__                

#include "ilist.hpp"


class CDirEntry : public CIndexedLink
{
     private:
          SDWORD resource_offset;
          SDWORD resource_size;

     public:
          CIndexedList subdir;
          CDirEntry (XString name, SDWORD offset, SDWORD size) :
		     CIndexedLink (name), resource_offset (offset),
		     resource_size (size) {}

          SDWORD Offset () {return resource_offset;}
          SDWORD Size ()   {return resource_size;  }
};

#endif                            
