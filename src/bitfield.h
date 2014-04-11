#ifndef __BITFIELD_H
#define __BITFIELD_H
/*-------------------------------------------------------------------------*\
  <bitfield.h> -- BitField header file

  Date      Programmer  Description
  03/24/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

class BitField
{
 public:
  BitField();
  BitField(const BitField &b);
  ~BitField();

  void clearAll();
  void clear(int index);
  int count(int start = 0, int end = -1) const;
  bool isSet(int index) const;
  int getMaxSet() const;
  void move(int index, int where);
  void remove(int index);
  void set(int index);
  bool toggle(int index);

  BitField operator&(const BitField &other) const;
  BitField &operator|=(const BitField &other);
  bool operator==(const BitField &other);
  BitField &operator=(const BitField &other);

 private:
  void expand(int newSize);

 private:
  unsigned int size;
  unsigned int *bits;
};

#endif
