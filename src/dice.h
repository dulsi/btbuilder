#ifndef __DICE_H
#define __DICE_H
/*-------------------------------------------------------------------------*\
  <dice.h> -- Dice header file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>

/*
 [dice]
 number: bits[5] (add 1)
 type: bits[3] {d2, d4, d6, d8, d10, d12, d20, d100}
*/

#define DICE_VALIDTYPES 8

class BTDice
{
 public:
  BTDice(BinaryReadFile &f);
  BTDice();

  IShort getNumber() const;
  IShort getType() const;
  void read(BinaryReadFile &f);
  void setNumber(IShort val);
  void setType(IShort val);

 private:
  IShort number;
  IShort type;

  static IShort validType[DICE_VALIDTYPES];
};

#endif

