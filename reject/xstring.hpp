#ifndef __INC_XSTRING_HPP__          
#define __INC_XSTRING_HPP__  
     
#include "memhandl.hpp"

class XString
{ 
     private:
          MemHandle s;                 
          size_t len;
          XString (const MemHandle &m);

     public:
          // ------------------------------------------------------------
          // - Constructors & destructor
          // ------------------------------------------------------------
          XString (const char *str = "");
          XString (const XString &str);
          ~XString ();

          XString &operator = (const XString &str);
          XString &operator = (const char *str);

          // ------------------------------------------------------------
          // - Overloaded operators
          // ------------------------------------------------------------
          operator const char * ()                   const;
          char        &operator[] (int idx); 
          const char  &operator[] (int idx)          const;
          XString     operator() (int from, int to)  const;
                  
          int operator == (const XString &s) const; 
          int operator != (const XString &s) const; 
          int operator >  (const XString &s) const; 
          int operator <  (const XString &s) const; 
          int operator >= (const XString &s) const; 
          int operator <= (const XString &s) const; 

          XString  operator + (const XString &s) const; 
          XString  &operator += (const XString &s);       
                  
          // ------------------------------------------------------------
          // - Member Functions
          // ------------------------------------------------------------
          const char *Buffer() const;
          size_t Len ()        const;
          void DelWS ();
          void ToUpper ();
          void ToLower ();
          int InString (const XString &sub) const;
};

#endif
