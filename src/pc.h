#ifndef __PC_H
#define __PC_H
/*-------------------------------------------------------------------------*\
  <pc.h> -- Player character header file

  Date      Programmer  Description
  03/11/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "combatant.h"
#include "display.h"
#include "xmlserializer.h"

class BTEquipment : public XMLObject
{
 public:
  BTEquipment() : id(BTITEM_NONE), equipped(BTITEM_NOTEQUIPPED), known(false), charges(0) {}
  virtual void serialize(ObjectSerializer* s);

  int id;
  int equipped;
  bool known;
  int charges;
};

class BTPc : public XMLObject, public BTCombatant
{
 public:
  BTPc();
  BTPc(int monsterType, int job);
  ~BTPc() { delete [] name; delete [] skill; }

  bool advanceLevel();
  void equip(int index);
  int incrementStat();
  bool isEquipped(int index) const;
  bool isEquipmentEmpty() const;
  bool isEquipmentFull() const;
  int getGold() const;
  int getHandWeapon() const;
  int getItem(int index) const;
  unsigned int getXPNeeded();
  unsigned int giveGold(unsigned int amount);
  void giveHP(int amount);
  bool giveItem(int id, bool known, int charges);
  void giveXP(unsigned int amount);
  bool hasItem(int id) const;
  bool savingThrow(int difficulty = BTSAVE_DIFFICULTY) const;
  virtual void serialize(ObjectSerializer* s);
  void setName(const char *nm);
  unsigned int takeGold(unsigned int amount);
  bool takeItem(int id);
  bool takeItemFromIndex(int index);
  void unequip(int index);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTPc*> &pc);
  static void writeXML(const char *filename, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int job;
  int picture;
  int monster;
  int stat[BT_STATS];
  int rateAttacks;
  int save;
  int sp, maxSp;
  int level;
  int criticalHit;
  unsigned int xp;
  unsigned int gold;
  int *skill;
  BTEquipment item[BT_ITEMS];

  // Combat actions
  class BTPcAction
  {
   public:
    enum pcAction { advance, attack, partyAttack, defend, useItem, runAway, cast, skill, npc };
    enum objectType { none, item, spell };

    void clearTarget(int group, int member = BTTARGET_INDIVIDUAL);
    void setTarget(int group, int member = BTTARGET_INDIVIDUAL);
    int getTargetGroup() const { return target >> BTTARGET_GROUPSHIFT; }
    int getTargetIndividual() const { return target & BTTARGET_INDIVIDUAL; }

    pcAction action;
    objectType type;
    int object;
    int target;
  };

  BTPcAction combat;
};

class BTParty : public XMLVector<BTPc*>
{
 public:
  void add(BTDisplay &d, BTPc *pc);
  bool checkDead(BTDisplay &d);
  void moveTo(int who, int where, BTDisplay &d);
  bool remove(int who, BTDisplay &d);

 private:
  BitField removing;
};

class BTStatusLookup : public ArrayLookup
{
 public:
  BTStatusLookup() : ArrayLookup(8, value) {}

  static BTStatusLookup lookup;

 private:
  static char *value[8];
};

#endif
