#ifndef __INC_BASE_HPP__
#define __INC_BASE_HPP__

#include "constant.hpp"

// *****************************************************************************
//
//  CLASS: CBase. This is the base class from which *ALL* other classes should
//	   be derived. This class is used to provide consistent error handling
//	   for all classes. If the constructor for a class fails in some way it
//	   should call one of the Fail () funtions below so that the validity
//	   of the class can be tested using the ValidObject function below.
//
//  PRIVATE MEMBERS: 
//        fValid      :  Flag to indicate the validity of the Object.
//
//  PUBLIC MEMBERS:
//        Valid ()    :  Function to test the validity of the object. Returns
//                       bTRUE if the object is valid or bFALSE if the object
//                       is invalid.
//  PROTECTED MEMBERS:
//        Fail ()     :  Function to invalidate the object. Can only be called
//			 from the derived classes and is typicaly called from
//			 the constructor if it fails in some way.
//
//        Fail (int)  :  Similar to the the above function but takes an int.
//			 If the int is >0 then the class will be invalidated.
//			 eg: 
//
//                            ptr = new char[200];
//                            Fail (ptr == NULL);
//
//                       will invalidate the object if the new fails.
//
// *****************************************************************************

class CBase
{
     private:
          BOOLEAN fValid;

     public:
          CBase () 
          { 
               fValid = bTRUE; 
          }

          virtual ~CBase () {;}
          BOOLEAN  Valid () {return fValid;}

     protected:
          void Fail () {fValid = bFALSE;}
          void Fail (int exp) {if (exp) fValid = bFALSE;}
};


// *****************************************************************************
//
//  FUNCTION: This function Validates an object which is derived from the 
//            class CBase. A Class can fail to initialise in 2 places. The 
//            memory for the class itself can fail to be allocated or something
//            the constructor does can fail. This function will test for both
//            cases. eg:
//
//                  ObjPrt = new TestObject;
//                  if (!ValidObject (ObjPtr)) 
//                  {
//                       -=-=- Handle error -=-=- 
//                  }
//
//  ENTRY:    A pointer to the object.
//  RETURNS:  bTRUE  if the object is valid 
//            bFALSE if the object is invalid and should not be used (and  
//                      should be deleted.)
//
// *****************************************************************************

inline BOOLEAN ValidObject (CBase *base)
{
     if ((base) && (base->Valid() == bTRUE))
          return (bTRUE);
     else
          return (bFALSE);
}

#endif
