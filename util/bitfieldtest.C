/*-------------------------------------------------------------------------*\
  <bitfieldtest.C> -- Unit testing for BitField

  Date      Programmer  Description
  09/20/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "bitfield.h"
#include <stdio.h>

void printBits(const char *label, BitField &b)
{
 printf("%s", label);
 for (int i = b.getMaxSet(); i >= 0; --i)
 {
  if (b.isSet(i))
   printf("1");
  else
   printf("0");
 }
 printf("\n");
}

int main(int argc, char *argv[])
{
 BitField small;
 BitField large;

 int i;
 for (i = (sizeof(unsigned int) << 3) - 1; i >= 0; i -= 2)
  small.set(i);
 for (i = ((sizeof(unsigned int) << 3) * 3) - 1; i >= 0; i -= 2)
  large.set(i);

 printf("unsigned int: %d\n", sizeof(unsigned int));
 printBits("Small:            ", small);
 printBits("Large: ", large);

 small.move(1, 0);
 printBits("Small.move(1,0):   ", small);
 small.move(5, 8);
 printBits("Small.move(5,8):   ", small);
 small.move(8, 5);
 printBits("Small.move(8,5):   ", small);
 small.move((sizeof(unsigned int) << 3) - 1, 5);
 printBits("Small.move(end,5): ", small);
 small.remove(12);
 printBits("Small.remove(12):  ", small);
 large.move(1, 0);
 printBits("Large.move(1,0):   ", large);
 large.move(5, 8);
 printBits("Large.move(5,8):   ", large);
 large.move(8, 5);
 printBits("Large.move(8,5):   ", large);
 large.move((sizeof(unsigned int) << 3) - 1, 5);
 printBits("Large.move(end first int,5): ", large);
 large.move((sizeof(unsigned int) << 3) * 2 + 4, (sizeof(unsigned int) << 3) * 1 + 4);
 printBits("Large.move(last int,middle int): ", large);
 large.remove(12);
 printBits("Large.remove(12):  ", large);
 large.remove(45);
 printBits("Large.remove(45):  ", large);
 return 0;
}
