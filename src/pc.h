#ifndef __PC_H
#define __PC_H
/*-------------------------------------------------------------------------*\
  <pc.h> -- Player character header file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "xmlserializer.h"

class BTEquipment : public XMLObject
{
 public:
  BTEquipment() : id(BTITEM_NONE) {}
  virtual void serialize(XMLSerializer* s);

  int id;
  bool equipped, known;
  int charges;
};

class BTPc : public XMLObject
{
 public:
  BTPc()
   : race(0), job(0), ac(0), hp(0), maxHp(0),  sp(0), maxSp(0), gold(0), xp(0)
  {
   name = new char[1];
   name[0] = 0;
   for (int i = 0; i < BT_STATS; ++i)
    stat[i] = 0;
  }

  ~BTPc() { delete [] name; }

  bool isAlive() const;
  bool isEquipped(int index) const;
  bool isEquipmentEmpty() const;
  bool isEquipmentFull() const;
  int getGold() const;
  int getItem(int index) const;
  bool giveItem(int id, bool known, int charges);
  void giveXP(int amount);
  bool hasItem(int id);
  virtual void serialize(XMLSerializer* s);
  void setName(const char *nm);
  int takeGold(int amount);
  bool takeItem(int id);

  static XMLObject *create() { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int stat[BT_STATS];
  int ac;
  int hp, maxHp;
  int sp, maxSp;
  int gold;
  unsigned int xp;
  BTEquipment item[BT_ITEMS];
};

#endif
