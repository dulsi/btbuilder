/*-------------------------------------------------------------------------*\
  <bitfield.C> -- BitField implementation file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "bitfield.h"

BitField::BitField()
 : size(0), bits(0)
{
}

BitField::BitField(const BitField &b)
 : size(b.size), bits(0)
{
 if (b.bits)
 {
  bits = new unsigned int[size];
  for (int i = 0; i < size; ++i)
   bits[i] = b.bits[i];
 }
}

BitField::~BitField()
{
 clearAll();
}

void BitField::clearAll()
{
 if (bits)
  delete [] bits;
 size = 0;
 bits = 0;
}

void BitField::clear(int index)
{
 if (bits)
 {
  int where = index / (sizeof(unsigned int) << 3);
  if (where < size)
   bits[where] &= (~(1 << (index % (sizeof(unsigned int) << 3))));
 }
 else
 {
  if (index < (sizeof(size) << 3))
   size &= (~(1 << index));
 }
}

int BitField::count(int start /*= 0*/, int end /*= -1*/) const
{
 if (end == -1)
  end = getMaxSet();
 int count = 0;
 while (start <= end)
 {
  if (isSet(start))
   ++count;
  ++start;
 }
 return count;
}

int BitField::getMaxSet() const
{
 if (bits)
 {
  for (int i = size - 1; i >= 0; --i)
  {
   if (0 != bits[i])
   {
    int k = 0;
    unsigned int value = bits[i];
    while (value > 0U)
    {
     ++k;
     value >>= 1;
    }
    return (k - 1) + (i * (sizeof(unsigned int) << 3));
   }
  }
 }
 else
 {
  int i = 0;
  unsigned int value = size;
  while (value > 0U)
  {
   ++i;
   value >>= 1;
  }
  return i - 1;
 }
 return -1;
}

bool BitField::isSet(int index) const
{
 if (bits)
 {
  int where = index / (sizeof(unsigned int) << 3);
  if (where < size)
   return (bits[where] & (1 << (index % (sizeof(unsigned int) << 3))));
 }
 else
 {
  if (index < (sizeof(size) << 3))
   return (size & (1 << index));
 }
 return false;
}

void BitField::move(int index, int where)
{
 if (index == where)
  return;
 if (0 == bits)
 {
  if (0 == size)
   return;
  else if ((index < (sizeof(size) << 3)) && (where < (sizeof(size) << 3)))
  {
   bool value = ((size & (1U << index)) != 0U);
   if (index > where)
   {
    unsigned int lowmask = (1U << where) - 1;
    unsigned int middlemask = ((1U << index) - 1) & ~lowmask;
    unsigned int highmask = ~((1U << (index + 1) ) - 1);
    if ((sizeof(size) << 3) == index + 1)
     highmask = 0;
    size = (size & lowmask) | ((size & middlemask) << 1) | (size & highmask);
   }
   else
   {
    unsigned int lowmask = (1U << index) - 1;
    unsigned int middlemask = ((1U << where) - 1) & ~((lowmask << 1) + 1);
    unsigned int highmask = ~((1U << (where + 1)) - 1);
    if ((sizeof(size) << 3) == where + 1)
     highmask = 0;
    size = (size & lowmask) | ((size & middlemask) >> 1) | (size & highmask);
   }
   if (value)
    size |= (1 << where);
  }
  else
  {
   if (index > where)
    expand(index / (sizeof(unsigned int) << 3) + 1);
   else
    expand(where / (sizeof(unsigned int) << 3) + 1);
  }
 }
 if (bits)
 {
  int pos1 = index / (sizeof(unsigned int) << 3);
  int pos2 = where / (sizeof(unsigned int) << 3);
  if ((pos1 >= size) && (pos2 >= size))
   return;
  else if (pos1 >= size)
   expand(pos1 + 1);
  else if (pos2 >= size)
   expand(pos2 + 1);
  bool value = ((bits[pos1] & (1U << (index % (sizeof(unsigned int) << 3)))) != 0U);
  if (pos1 == pos2)
  {
   if (index > where)
   {
    unsigned int lowmask = (1U << (where % (sizeof(unsigned int) << 3))) - 1;
    unsigned int middlemask = ((1U << (index % (sizeof(unsigned int) << 3))) - 1) & ~lowmask;
    unsigned int highmask = ~((1U << ((index + 1) % (sizeof(unsigned int) << 3))) - 1);
    if ((sizeof(size) << 3) == index + 1)
     highmask = 0;
    bits[pos1] = (bits[pos1] & lowmask) | ((bits[pos1] & middlemask) << 1) | (bits[pos1] & highmask);
   }
   else
   {
    unsigned int lowmask = (1U << (index % (sizeof(unsigned int) << 3))) - 1;
    unsigned int middlemask = ((1U << (where % (sizeof(unsigned int) << 3))) - 1) & ~((lowmask << 1) + 1);
    unsigned int highmask = ~((1U << ((where + 1) % (sizeof(unsigned int) << 3))) - 1);
    if ((sizeof(size) << 3) == where + 1)
     highmask = 0;
    bits[pos1] = (bits[pos1] & lowmask) | ((bits[pos1] & middlemask) >> 1) | (bits[pos1] & highmask);
   }
  }
  else if (pos1 < pos2)
  {
   unsigned int lowmask = (1U << (index % (sizeof(unsigned int) << 3))) - 1;
   unsigned int highmask = ~((1U << (index % (sizeof(unsigned int) << 3))) | lowmask);
   bits[pos1] =  ((bits[pos1] & highmask) >> 1) | (bits[pos1] & lowmask);
   for (++pos1; pos1 < pos2; ++pos1)
   {
    if (bits[pos1] & 1)
     bits[pos1 - 1] |= 1 << ((sizeof(unsigned int) << 3) - 1);
    bits[pos1] >>= 1;
   }
   if (bits[pos2] & 1)
    bits[pos2 - 1] |= 1 << ((sizeof(unsigned int) << 3) - 1);
   lowmask = (1U << (where % (sizeof(unsigned int) << 3))) - 1;
   highmask = ~lowmask;
   bits[pos2] = (bits[pos2] & highmask) | ((bits[pos2] & lowmask) >> 1);
  }
  else
  {
   unsigned int lowmask = (1U << (index % (sizeof(unsigned int) << 3))) - 1;
   unsigned int highmask = ~((1U << (index % (sizeof(unsigned int) << 3))) | lowmask);
   bits[pos1] =  (bits[pos1] & highmask) | ((bits[pos1] & lowmask) << 1);
   for (--pos1; pos1 > pos2; --pos1)
   {
    if (bits[pos1] & (1 << ((sizeof(unsigned int) << 3) - 1)))
     bits[pos1 - 1] |= 1;
    bits[pos1] <<= 1;
   }
   if (bits[pos2] & (1 << ((sizeof(unsigned int) << 3) - 1)))
    bits[pos2 - 1] |= 1;
   lowmask = (1U << (where % (sizeof(unsigned int) << 3))) - 1;
   highmask = ~lowmask;
   bits[pos2] = ((bits[pos2] & highmask) << 1) | (bits[pos2] & lowmask);
  }
  if (value)
   bits[pos2] |= (1 << (where % (sizeof(unsigned int) << 3)));
 }
}

void BitField::remove(int index)
{
 if (bits)
 {
  int where = index / (sizeof(unsigned int) << 3);
  if (where < size)
  {
   unsigned int lowmask = (1U << (index % (sizeof(unsigned int) << 3))) - 1;
   unsigned int highmask = ~((1U << (index % (sizeof(unsigned int) << 3))) | lowmask);
   bits[where] =  ((bits[where] & highmask) >> 1) | (bits[where] & lowmask);
   for (++where; where < size; ++where)
   {
    if (bits[where] & 1)
     bits[where - 1] |= 1 << ((sizeof(unsigned int) << 3) - 1);
    bits[where] >>= 1;
   }
  }
 }
 else
 {
  if (index < (sizeof(size) << 3))
  {
   unsigned int lowmask = (1U << index) - 1;
   unsigned int highmask = ~((1U << index) | lowmask);
   size =  ((size & highmask) >> 1) | (size & lowmask);
  }
 }
}

void BitField::set(int index)
{
 if (0 == bits)
 {
  if (index < (sizeof(size) << 3))
   size |= (1 << index);
  else
   expand(index / (sizeof(unsigned int) << 3) + 1);
 }
 if (bits)
 {
  int where = index / (sizeof(unsigned int) << 3);
  if (where >= size)
   expand(where + 1);
  bits[where] |= (1 << (index % (sizeof(unsigned int) << 3)));
 }
}

bool BitField::toggle(int index)
{
 if (isSet(index))
 {
  clear(index);
  return false;
 }
 else
 {
  set(index);
  return true;
 }
}

void BitField::expand(int newSize)
{
 if (bits)
 {
  unsigned int *oldbits = bits;
  bits = new unsigned int[newSize];
  int i;
  for (i = 0; i < size; ++i)
   bits[i] = oldbits[i];
  for (; i < newSize; ++i)
   bits[i] = 0;
  delete [] oldbits;
 }
 else
 {
  bits = new unsigned int[newSize];
  bits[0] = size;
  for (int i = 1; i < newSize; ++i)
   bits[i] = 0;
 }
 size = newSize;
}

BitField BitField::operator&(const BitField &other) const
{
 BitField ans;
 if (bits)
 {
  if (other.bits)
  {
   int i = size;
   if (other.size < i)
    i = other.size;
   ans.expand(size);
   for (--i; i >= 0; --i)
    ans.bits[i] = bits[i] & other.bits[i];
  }
  else
   ans.size = bits[0] & other.size;
 }
 else if (other.bits)
 {
  ans.size = size & other.bits[0];
 }
 else
 {
  ans.size = size & other.size;
 }
 return ans;
}

BitField &BitField::operator|=(const BitField &other)
{
 if (bits)
 {
  if (other.bits)
  {
   if (other.size > size)
    expand(other.size);
   for (int i = 0; i < size; ++i)
    bits[i] |= other.bits[i];
  }
  else
   bits[0] |= other.size;
 }
 else if (other.bits)
 {
  expand(other.size);
  for (int i = 0; i < size; ++i)
   bits[i] |= other.bits[i];
 }
 else
 {
  size |= other.size;
 }
 return *this;
}

bool BitField::operator==(const BitField &other)
{
 int len = getMaxSet();
 if (other.getMaxSet() != len)
  return false;
 for (; len > 0; --len)
  if (isSet(len - 1) != other.isSet(len - 1))
   return false;
 return true;
}

BitField &BitField::operator=(const BitField &other)
{
 if (this != &other)
 {
  clearAll();
  size = other.size;
  if (other.bits)
  {
   bits = new unsigned int[size];
   for (int i = 0; i < size; ++i)
    bits[i] = other.bits[i];
  }
 }
 return *this;
}

