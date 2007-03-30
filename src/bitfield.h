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

  bool isSet(int index) const;
  void set(int index);

 private:
  int bits;
};

#endif
