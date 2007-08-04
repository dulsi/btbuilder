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

  void equip(int index);
  bool isAlive() const;
  bool isEquipped(int index) const;
  bool isEquipmentEmpty() const;
  bool isEquipmentFull() const;
  int getGold() const;
  int getItem(int index) const;
  unsigned int giveGold(unsigned int amount);
  void giveHP(int amount);
  bool giveItem(int id, bool known, int charges);
  void giveXP(unsigned int amount);
  bool hasItem(int id) const;
  bool savingThrow(int difficulty) const;
  virtual void serialize(ObjectSerializer* s);
  void setName(const char *nm);
  unsigned int takeGold(unsigned int amount);
  bool takeHP(int amount);
  bool takeItem(int id);
  bool takeItemFromIndex(int index);
  void unequip(int index);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int picture;
  int monster;
  int stat[BT_STATS];
  int ac;
  int toHit;
  int save;
  BitField status;
  int hp, maxHp;
  int sp, maxSp;
  int level;
  unsigned int xp;
  unsigned int gold;
  int *spellLvl;
  BTEquipment item[BT_ITEMS];

  // Combat actions
  class BTPcAction
  {
   public:
    enum pcAction { advance, attack, partyAttack, defend, useItem, runAway };

    void clearTarget(int group, int member = BTTARGET_INDIVIDUAL);
    void setTarget(int group, int member = BTTARGET_INDIVIDUAL);
    int getTargetGroup() const { return target >> BTTARGET_GROUPSHIFT; }
    int getTargetIndividual() const { return target & BTTARGET_INDIVIDUAL; }

    bool active;
    pcAction action;
    int item;
    int target;
  };

  BTPcAction combat;
};

class BTParty : public XMLVector<BTPc*>
{
 public:
  bool checkDead();
};

class BTStatusLookup : public ArrayLookup
{
 public:
  BTStatusLookup() : ArrayLookup(7, value) {}

  static BTStatusLookup lookup;

 private:
  static char *value[7];
};

#endif
