#ifndef __STRUCTS_HPP__                
#define __STRUCTS_HPP__                

#include "portab.hpp"

#pragma pack(2)                    // switch to word struct packing

struct PWAD_HEADER
{
     SBYTE  signature[4];           // "PWAD" or "IWAD"     
     SDWORD dir_entries;
     SDWORD dir_offset;
};

struct DIR_ENTRY
{
     SDWORD resource_offset;
     SDWORD resource_size;
     SBYTE resource_name[8];
};

struct LINEDEF
{
     WORD from_vertex;
     WORD to_vertex;
     WORD attrib;
     WORD type;
     WORD trigger_tag;
     WORD sidedef1;
     WORD sidedef2;
};

struct SIDEDEF
{
     SWORD x_offset;
     SWORD y_offset;
     SBYTE upper_name[8];
     SBYTE lower_name[8];
     SBYTE normal_name[8];
     WORD sector;
};

struct VERTEX
{
     SWORD x;
     SWORD y;
};

struct SECTOR
{
     SWORD floor_pos;
     SWORD ceiling_pos;
     SBYTE floor_name[8];
     SBYTE ceiling_name[8];
     SWORD light_level;
     SWORD type;
     SWORD trigger_tag;
};

#pragma pack()                    // switch to default struct packing

#endif                            
