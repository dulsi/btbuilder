#ifndef __ITEM_H
#define __ITEM_H
/*-------------------------------------------------------------------------*\
  <item.h> -- Item header file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "dice.h"
#include "bitfield.h"
#include "factory.h"

class BTPc;

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

class BTItem : public XMLObject
{
 public:
  BTItem(BinaryReadFile &f);
  BTItem();
  BTItem(const BTItem &copy);
  ~BTItem();

  bool canUse(BTPc *pc) const;
  const std::string &getName() const;
  IShort getArmorPlus() const;
  const char *getCause() const;
  IShort getChanceXSpecial() const;
  const BTDice &getDamage() const;
  const char *getEffect() const;
  IShort getHitPlus() const;
  IShort getPrice() const;
  IShort getSpellCast() const;
  IShort getTimesUsable() const;
  IShort getType() const;
  IShort getXSpecial() const;
  bool isConsumed() const;
  void write(BinaryWriteFile &f);

  virtual void serialize(ObjectSerializer* s);
  void upgrade() {}

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTItem; }
  static void readXML(const char *filename, XMLVector<BTItem*> &item);
  static void writeXML(const char *filename, XMLVector<BTItem*> &item);

 private:
  std::string name;
  IShort timesUsable;
  BTDice damage;
  IShort armorPlus;
  IShort hitPlus;
  int xSpecial;
  IShort chanceXSpecial;
  int type;
  int spellCast;
  BitField classAllowed;
  IShort price;
  char *cause;
  char *effect;
  bool consume;

  static int compatJobAllowed[11];
};

class BTItemListCompare : public BTSortCompare<BTItem>
{
 public:
  int Compare(const BTItem &a, const BTItem &b) const;
};

#endif

