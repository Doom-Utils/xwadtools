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
#include "xstring.hpp"
#include "strfunc.h"


// *****************************************************************************
//
//  CONSTRUCTORS:   
//
// *****************************************************************************

XString::XString (const char *str)
        : s(strlen (str)+1), len (strlen (str))
{
     strcpy (s, str);
}

XString::XString (const XString& str)
        : s (str.s), len (str.len)
{
}

XString::XString (const MemHandle &m)
        : s (m), len (m.Size())
{
}

// *****************************************************************************
//
//  DESTRUCTOR:   
//
// *****************************************************************************

XString::~XString ()
{
}


// *****************************************************************************
//
//  FUNCTIONS:   
//
//  RETURNS:   
//
// *****************************************************************************

XString &XString::operator= (const XString &str)
{
     s = str.s;
     len = str.len;
     return *this;
}

XString &XString::operator= (const char *str)
{
     len = strlen (str);
     s = MemHandle (len+1);
     strcpy (s, str);
     return *this;
}


// *****************************************************************************
//
//  FUNCTION:   
//
//  RETURNS:   
//
// *****************************************************************************

XString::operator const char * () const
{
     return (s.Buffer());
}

// *****************************************************************************
//
//  FUNCTION:   
//
//  RETURNS:   
//
// *****************************************************************************

size_t XString::Len () const
{
     return len;
}

// *****************************************************************************
//
//  FUNCTIONS: Comparison Operators.  
//
//             Implementations of the following comparison operators:
//
//             ==, !=, >, <, >=, <= 
//
//  RETURNS:  1 if the comparison is true else 0 
//
// *****************************************************************************

int XString::operator == (const XString &str) const
{
     return (stricmp (s.Buffer(), str.s.Buffer()) == 0);
}
 
int XString::operator != (const XString &str) const 
{
     return (stricmp (s.Buffer(), str.s.Buffer()) != 0);
}

int XString::operator > (const XString &str) const 
{
     return (stricmp (s.Buffer(), str.s.Buffer()) > 0);
}

int XString::operator < (const XString &str) const 
{
     return (stricmp (s.Buffer(), str.s.Buffer()) < 0);
}

int XString::operator >= (const XString &str) const 
{            
     return (stricmp (s.Buffer(), str.s.Buffer()) >= 0);
}

int XString::operator <= (const XString &str) const 
{
     return (stricmp (s.Buffer(), str.s.Buffer()) <= 0);
}


// *****************************************************************************
//
//  FUNCTION:   
//
//  RETURNS:   
//
// *****************************************************************************

const char* XString::Buffer() const
{
     return (s.Buffer());
}
                                   

// *****************************************************************************
//
//  FUNCTIONS:   
//
//  RETURNS:   
//
// *****************************************************************************

XString  XString::operator + (const XString &str) const 
{                                              
     // -=- Get memory for new string -=-
     MemHandle temp (len + str.len + 1);
                         
     // -=- join the 2 strings -=-                    
     strcpy (temp, s.Buffer());
     strcat (temp, str.s.Buffer()); 
                    
     // -=- make new string to return -=- 
     return XString (temp);     
}
     

XString &XString::operator += (const XString &str)       
{
     // -=- extend the length of this string -=-
     len += str.len;

     // -=- Get memory for new string -=-
     MemHandle temp (len + 1);
     
     // -=- join the 2 strings -=-                    
     strcpy (temp, s.Buffer());
     strcat (temp, str.s.Buffer()); 

     s = temp;

     return *this;
}


// *****************************************************************************
//
//  FUNCTION: XString XString::operator() (int from, int to) const
//            return a substring from a string.
//
//  ENTRY: from = index of starting character of substring
//           to = index of ending character 
//
//  RETURNS:  the substring 
//
// *****************************************************************************

XString XString::operator () (int from, int to) const
{
     // -=- check for indexex out of range -=-
     if (((unsigned)from >= len) || (from > to))
          return XString();
                          
     // -=- if 'to' is past the end of the string then set to end -=-
     if ((unsigned)to >= len)
          to = len - 1;
     
     // -=- create buffer to hold string -=-
     MemHandle mh (to - from + 2);    
     
     // -=- copy substring into buffer -=-
     strncpy (mh, &(*this)[from], to - from+1);
     mh[to - from + 1] = 0;
     
     // -=- return substring -=- 
     return XString (mh);
}

// *****************************************************************************
//
//  FUNCTION:   
//
//  RETURNS:   
//
// *****************************************************************************

const char &XString::operator [] (int idx) const
{
     static char dummy = 0;

     if ((unsigned)idx >= len)
          return dummy;
     else
          return s.Buffer()[idx];
}

char &XString::operator [] (int idx)
{
     static char dummy;

     if ((unsigned)idx >= len)
     {
          dummy = 0;
          return dummy;
     }
     else
     {
          if (s.RefCount() > 1)
               s = s.Clone();
          return s[idx];
     }
}

// *****************************************************************************
//
//  FUNCTION:   
//
//  RETURNS:   
//
// *****************************************************************************

void XString::DelWS ()
{
     if (s.RefCount() > 1)
          s = s.Clone();    
     
     char *start = s;
     for(char *x=start+len; (*x<=' ')&&(x >= start) ; *x-- ='\0')
          ;
        
     len = strlen (s);
}


void XString::ToUpper ()
{
     if (s.RefCount() > 1)
          s = s.Clone();    
     strupr (s);
}

void XString::ToLower ()
{
     if (s.RefCount() > 1)
          s = s.Clone();    
     strlwr (s);
}

int XString::InString (const XString &sub) const
{
     char *x = strstr (s.Buffer(), sub.s.Buffer());     

     if (x == 0)
          return (-1);
     else
          return (x - s.Buffer());
}


// *****************************************************************************
// *****************************************************************************
// *
// *      Test rig for the XString class
// *
// *****************************************************************************
// *****************************************************************************
// #define _TEST_XSTRING_

#ifdef _TEST_XSTRING_

#include <stdio.h>

// *****************************************************************************
// -=- Over loaded new & delete to track free store leeks during testing -=-
// *****************************************************************************

void * operator new (size_t s)
{    
     void *p = malloc (s);
          
     printf ("New: Allocate %04u bytes @%p\n", s, p);
     return p;
}

void operator delete (void *s)
{
     printf ("Delete @%p\n", s);
     free (s);
}


void main ()
{
     XString s1 ("aaaaaaaaaa");
     XString s2 ("bbbbbbbbbb");
     XString s3 (s1);

     printf (" s1 %s\n", s1.Buffer());
     printf (" s2 %s\n", s2.Buffer());
     printf (" s3 %s\n", s3.Buffer());


     printf (" s3 == s2 %d\n", s3 == s2);
     printf (" s2 != s3 %d\n", s2 != s3);

     printf (" s1 > s2 %d\n", s1 > s2);
     printf (" s2 > s1 %d\n", s2 > s1);

     printf (" s1 < s2 %d\n", s1 < s2);
     printf (" s2 < s1 %d\n", s2 < s1);

     printf (" s3 >= s1 %d\n", s3 >= s1);
     printf (" s3 <= s1 %d\n", s3 <= s1);
     
     XString s4;
     
     s4 = s1 + s2;
     printf (" s4 %s\n", s4.Buffer());
     s4 += s2;
     printf (" s4 %s\n", s4.Buffer());

     s4 = "wibble ";
     s4 = s4 + s4;
     printf (" s4 %s\n", s4.Buffer());
     s4 += s4;
     printf (" s4 %s\n", s4.Buffer()); 
     
     s4 = s4;

     XString s5 = "0123456789";
     XString s6 = s5 (10, 60);
     puts (s6);

     XString s7 = s6 = "HELLO WORLD!!";

     for (int n = 0; n < 53; n++)
          s6[n] = 'x';

     puts (s7);

     char a = s7[4]; 
     
     
     XString s8 = "HeLlO 2 yOu                ";
     
     s8.ToUpper ();
     puts (s8);    
     s8.ToLower ();
     puts (s8);
     
     s8.DelWS();
     printf (">%s<\n", s8.Buffer()); 

     printf ("%d\n", s8.InString ("you"));
     printf ("%d\n", s8.InString ("wibble"));
}

#endif
