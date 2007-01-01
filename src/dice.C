/*-------------------------------------------------------------------------*\
  <dice.C> -- Dice implementation file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "dice.h"

#define DICE_NUMBERMASK 0x1F
#define DICE_TYPEMASK   0xE0

IShort BTDice::validType[DICE_VALIDTYPES] = {2, 4, 6, 8, 10, 12, 20, 100};

BTDice::BTDice(BinaryReadFile &f)
{
 read(f);
}

BTDice::BTDice()
 : number(1), type(validType[0])
{
}

IShort BTDice::getNumber() const
{
 return number;
}

IShort BTDice::getType() const
{
 return type;
}

void BTDice::read(BinaryReadFile &f)
{
 IUByte b;

 f.readUByte(b);
 number = (b & DICE_NUMBERMASK) + 1;
 type = validType[(b & DICE_TYPEMASK) >> 5];
}

void BTDice::setNumber(IShort val)
{
 number = val;
}

void BTDice::setType(IShort val)
{
 int i;
 for (i = 0; i < DICE_VALIDTYPES; i++)
 {
  if (val == validType[i])
  {
   type = val;
   return;
  }
 }
 // Error tried to set an invalid type
}

void BTDice::write(BinaryWriteFile &f)
{
 IUByte b;

 for (b = 0; b < DICE_VALIDTYPES; b++)
 {
  if (type == validType[b])
  {
   break;
  }
 }
 b = (b << 5) + number;
 f.writeUByte(b);
}

