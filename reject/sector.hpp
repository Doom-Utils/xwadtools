#ifndef __SECTOR_HPP__                
#define __SECTOR_HPP__                

#include "structs.hpp"
#include "base.hpp"

class CSector : public CBase
{
     private:
          SWORD max_x;
          SWORD max_y;
          SWORD min_x;
          SWORD min_y;

     public:
          CSector ();
          void AddVertex (VERTEX &v);
          void GetCenter (SWORD &x, SWORD &y);
};

#endif                            
