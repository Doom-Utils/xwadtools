#ifndef __INC_LIST_HPP__
#define __INC_LIST_HPP__

#include "base.hpp"

// *****************************************************************************
//
//  CLASS: CLink : A link in a doubly linked list. Use this class as a 
//                 base class to make linked lists of other classes.
//
//  PRIVATE MEMBERS:
//        next = pointer to next CLink in the list or NULL if last in list
//        prev = pointer to previous CLink in the list or NULL if first in list
//
//  PUBLIC MEMBERS:
//        Next ()    = Returns a pointer to the next link in chain or NULL if 
//                     last link in chain.
//        Prev ()    = Returns a pointer to the previous link in chain or NULL
//		       if first link in chain.
//
// *****************************************************************************

class CLink : public CBase
{
     friend class CList; 
     protected:
          CLink *next;
          CLink *prev;

     public:
          CLink () : next(0), prev(0) {;}
          CLink *Next () { return (next); }
          CLink *Prev () { return (prev); }
          virtual ~CLink () {;}
};


// *****************************************************************************
//
//  CLASS: CList : A Doubly link list class
//
//  PRIVATE MEMBERS:
//        _base = pointer to first CLink in the list or NULL if no items in list
//
//  PUBLIC MEMBERS:
//        AddItem ()   = Adds a new CLink Object to the begining of the list.
//        FirstItem () = Returns a pointer to the first CLink in the list.  
//        DeleteItem ()= deletes an item and removes it from the list.  
//        DeleteAll () = deletes all the items in the list.  
//
// *****************************************************************************

class CList : public CBase
{
     protected:
          CLink *base;               

     public:
          CList () { base = 0; }
          void AddItem (CLink *link);
          void DeleteItem (CLink *link);
          void DeleteAll ();
          CLink *FirstItem () { return (base); }
          ~CList ();
};

#endif
