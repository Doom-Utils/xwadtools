#ifndef __REJECT_HPP__                
#define __REJECT_HPP__                

#include "pwad.hpp"
#include "sector.hpp"

class CReject : public CBase
{
     private:
          int numsectors;
          CPWad &pwad;
          void BuildSectorArray ();
          CSector *sect;
          MemHandle rejectdata;
          MemHandle linedefdata;
          MemHandle sidedefdata;
          MemHandle vertexdata;
          MemHandle sectordata;
          LINEDEF * linedef;
          SIDEDEF * sidedef;
          VERTEX  * vertex;
          SECTOR  * sector;

          void SetBit (char* buffer, int bit);
          void ResetBit (char* buffer, int bit);
          int  idist (int x1, int y1, int x2, int y2);

     public:
          CReject (CPWad &w, XString map);
          ~CReject ();

          void RejectDefault (int threshold);
          void RejectSector  (int local, int remote);
          void AcceptSector  (int local, int remote);
          void RejectAll     ();
          void AcceptAll     ();
          MemHandle GetRejectData ();
};

#endif                            
