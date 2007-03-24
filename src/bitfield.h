#ifndef __BITFIELD_H
#define __BITFIELD_H
/*-------------------------------------------------------------------------*\
  <bitfield.h> -- BitField header file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <string>

class BitField
{
 public:
  BitField();

  bool isSet(int index);
  void set(int index);

 private:
  int bits;
};

class BitFieldLookup
{
 public:
  virtual std::string getName(int index) = 0;
  virtual int getIndex(std::string name) = 0;
};

#endif
