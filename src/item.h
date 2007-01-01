#ifndef __ITEM_H
#define __ITEM_H
/*-------------------------------------------------------------------------*\
  <item.h> -- Item header file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "dice.h"

/*
[item]
name: char[25]
unknown: byte
times usable: short {0 to 255}
damage: dice
unknown: byte
armor plus: short {-15 to 15}
hit plus: short {-15 to 15}
x-special: short
chance x-special: short {0 to 100}
type: short
spell cast: short
class allowed: short
price: short {0 to 32000}
cause: char[24]
effect: char[24]
*/

class BTItem
{
 public:
  BTItem(BinaryReadFile &f);
  BTItem();

  const char *getName() const;
  IShort getArmorPlus() const;
  const char *getCause() const;
  IShort getChanceXSpecial() const;
  const BTDice &getDamage() const;
  const char *getEffect() const;
  IShort getHitPlus() const;
  IShort getPrice() const;
  IShort getTimesUsable() const;
  IShort getType() const;
  IShort getXSpecial() const;

 private:
  char name[25];
  IShort timesUsable;
  BTDice damage;
  IShort armorPlus;
  IShort hitPlus;
  IShort xSpecial;
  IShort chanceXSpecial;
  IShort type;
  IShort spellCast;
  IShort classAllowed;
  IShort price;
  char cause[24];
  char effect[24];
};

#endif

