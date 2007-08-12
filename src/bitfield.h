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

  void clearAll();
  void clear(int index);
  bool isSet(int index) const;
  int getMaxSet() const;
  void set(int index);

 private:
  int bits;
};

#endif
