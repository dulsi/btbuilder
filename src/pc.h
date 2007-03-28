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
  virtual void serialize(ObjectSerializer* s);

  int id;
  bool equipped, known;
  int charges;
};

class BTPc : public XMLObject
{
 public:
  BTPc();
  ~BTPc() { delete [] name; delete [] spellLvl; }

  bool isAlive() const;
  bool isEquipped(int index) const;
  bool isEquipmentEmpty() const;
  bool isEquipmentFull() const;
  int getGold() const;
  int getItem(int index) const;
  unsigned int giveGold(unsigned int amount);
  bool giveItem(int id, bool known, int charges);
  void giveXP(unsigned int amount);
  bool hasItem(int id);
  virtual void serialize(ObjectSerializer* s);
  void setName(const char *nm);
  unsigned int takeGold(unsigned int amount);
  bool takeItem(int id);

  static XMLObject *create() { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int picture;
  int monster;
  int stat[BT_STATS];
  int ac;
  int hp, maxHp;
  int sp, maxSp;
  int level;
  unsigned int xp;
  unsigned int gold;
  int *spellLvl;
  BTEquipment item[BT_ITEMS];
};

#endif
