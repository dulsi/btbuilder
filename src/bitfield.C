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

bool BitField::isSet(int index)
{
 return ((bits & (1 << index)) != 0);
}

void BitField::set(int index)
{
 bits |= (1 << index);
}
