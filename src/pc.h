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
#include "group.h"
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

class BTSkillValue : public XMLObject
{
 public:
  BTSkillValue() : skill(0), value(0), uses(0) {}
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSkillValue; }

  int skill;
  int value;
  int uses;
  std::vector<unsigned int> history;
};

class BTPc : public XMLObject, public BTCombatant
{
 public:
  BTPc();
  BTPc(int monsterType, int job, BTCombatant *c = NULL);
  ~BTPc() { delete [] name; delete [] item; }

  bool advanceLevel();
  bool age();
  std::string attack(BTCombatant *defender, int weapon, int &numAttacksLeft, int &activeNum);
  void changeJob(int newJob);
  bool drainItem(int amount);
  bool drainLevel();
  void equip(int index);
  int hiddenTime() const;
  int incrementStat();
  bool isEquipped(int index) const;
  bool isEquipmentEmpty() const;
  bool isEquipmentFull() const;

  std::string getName() const;
  int getGender() const;
  int getGold() const;
  int getHandWeapon() const;
  int getItem(int index) const;
  int getSkill(int skNum) const;
  unsigned int getXPNeeded();
  unsigned int giveGold(unsigned int amount);
  void giveHP(int amount);
  bool giveItem(int id, bool known, int charges);
  void giveSP(int amount);
  void giveSkillUse(int skNum, int amount);
  void giveXP(unsigned int amount);
  bool hasItem(int id) const;
  bool hasSkillUse(int skNum);
  bool isIllusion() const;
  virtual void restoreLevel();
  virtual void rollInitiative();
  bool savingThrow(int difficulty = BTSAVE_DIFFICULTY) const;
  virtual void serialize(ObjectSerializer* s);
  void setName(const char *nm);
  void setSkill(int skNum, int value, int uses);
  unsigned int takeGold(unsigned int amount);
  bool takeItem(int id);
  bool takeItemFromIndex(int index);
  void takeItemCharge(int index, int amount = 1);
  bool takeSP(int amount);
  void unequip(int index);
  void useAutoCombatSkill(bool melee, BitField &special);
  bool useSkill(int index, int difficulty = BTSKILL_DEFAULTDIFFICULTY);
  void updateSkills();
  void youth();

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPc; }
  static void readXML(const char *filename, XMLVector<BTGroup*> &group, XMLVector<BTPc*> &pc);
  static void writeXML(const char *filename, XMLVector<BTGroup*> &group, XMLVector<BTPc*> &pc);

  char *name;
  int race;
  int gender;
  int picture;
  int monster;
  int stat[BT_STATS];
  int statMax[BT_STATS];
  int rateAttacks;
  int save;
  int sp, maxSp;
  unsigned int xp;
  unsigned int gold;
  XMLVector<BTSkillValue*> skill;
  BTEquipment *item;

  // Combat actions
  class BTPcAction
  {
   public:
    enum pcAction { advance, attack, partyAttack, defend, useItem, runAway, cast, useSkill, sing, npc };
    enum objectType { none, item, spell, song, skill };

    BTPcAction() : skillUsed(-1), consecutiveUsed(0) { }

    void clearSkillUsed() { skillUsed = -1; consecutiveUsed = 0; }
    void setSkillUsed(int skill) { if (skillUsed == skill) { ++consecutiveUsed; } else { skillUsed = skill; consecutiveUsed = 1; } }
    void clearTarget(int group, int member = BTTARGET_INDIVIDUAL);
    void setTarget(int group, int member = BTTARGET_INDIVIDUAL);
    int getTargetGroup() const { return target >> BTTARGET_GROUPSHIFT; }
    int getTargetIndividual() const { return target & BTTARGET_INDIVIDUAL; }

    pcAction action;
    objectType type;
    int object;
    int target;
    int skillUsed;
    int consecutiveUsed;
  };

  BTPcAction combat;
};

class BTParty : public XMLVector<BTPc*>, public BTCombatantCollection
{
 public:
  BTParty() : XMLVector<BTPc*>(false) {}

  void add(BTDisplay &d, BTPc *pc);
  bool checkDead(BTDisplay &d);
  void giveItem(int itemID, BTDisplay &d);
  void moveTo(int who, int where, BTDisplay &d);
  bool remove(int who, BTDisplay &d);

  BTCombatant* at(size_t index);
  int getDistance();
  size_t size();

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
