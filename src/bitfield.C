/*-------------------------------------------------------------------------*\
  <bitfield.C> -- BitField implementation file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "bitfield.h"

BitField::BitField()
 : bits(0)
{
}

void BitField::clearAll()
{
 bits = 0;
}

void BitField::clear(int index)
{
 bits &= (~(1 << index));
}

int BitField::getMaxSet() const
{
 int i = 0;
 while ((bits >> i) > 0)
  ++i;
 return i - 1;
}

bool BitField::isSet(int index) const
{
 return ((bits & (1 << index)) != 0);
}

void BitField::set(int index)
{
 bits |= (1 << index);
}
