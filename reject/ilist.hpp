#ifndef __INC_ILIST_HPP__
#define __INC_ILIST_HPP__

#include "list.hpp"
#include "xstring.hpp"


// *****************************************************************************
//
//  CLASS: CIndexedLink : A link in a doubly linked list. Use this class as a 
//                 base class to make linked lists of other classes.
//
//  PRIVATE MEMBERS:
//        key = 
//   
//  PUBLIC MEMBERS:
//
//
// *****************************************************************************

class CIndexedLink : public CLink
{
     friend class CIndexedList; 
     private:
          XString key;
          
     public:     
          CIndexedLink (const XString s) : key(s) {;}

          CIndexedLink *Next () { return ((CIndexedLink *)next); }
          CIndexedLink *Prev () { return ((CIndexedLink *)prev); }

          const XString GetKey () const { return key; }
          int operator>  (const CIndexedLink& l) const {return (key > l.key);} 
          int operator== (const CIndexedLink& l) const {return (key == l.key);} 
};


// *****************************************************************************
//
//  CLASS: CList : An Indexed Doubly link list class
//
//  PRIVATE MEMBERS:
//        base = pointer to first CIndexedLink in the list or NULL if no items
//		 in list
//
//  PUBLIC MEMBERS:
//        AddItem ()   = Adds a new CIndexedLink Item to the begining of the
//			 list
//        FirstItem () = Returns a pointer to the first CIndexedLink in the list
//
// *****************************************************************************

class CIndexedList : public CList
{
     public:
          CIndexedList () { base = 0; }
          void AddItem (CIndexedLink *link);
          CIndexedLink *FirstItem () { return ((CIndexedLink *)base); }
          CIndexedLink *operator[] (XString key);
};

#endif
