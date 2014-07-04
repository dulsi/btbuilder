#ifndef __MONSTER_H
#define __MONSTER_H
/*-------------------------------------------------------------------------*\
  <monster.h> -- Monster header file

  Date      Programmer  Description
  11/06/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "dice.h"
#include "btconst.h"
#include "combatant.h"
#include "display.h"
#include "factory.h"

/*
 [monster]
 name: char[14]
 starting distance: short {(1 to 9) * 10}
 move: short {0 to 15}
 rate of attacks: short {0 to 15}
 illusion: short {0 or 1}
 picture: short {0 to 74}
 combat action: array[4] of short
 melee extra damage: short
 armor class: short {-21 to 10}
 max appearing: short {1 to 99}
 hit point: dice
 melee damage: dice
 melee message: char[14]
 magic resistance: short {0 to 100}
 ranged damage: dice
 unknown: byte
 ranged extra damage: short
 range: short {1 to 15}
 ranged message: char[14]
 ranged type: short {none, physical: 1 foe, physical: group, magic (+ spell index)}
 level: short {0 to 20}
 gold: dice
 unknown: byte
*/

class BTMonster : public XMLObject
{
 public:
  BTMonster(BinaryReadFile &f);
  BTMonster();
  BTMonster(const BTMonster &copy);
  ~BTMonster();

  unsigned int calcXp() const;
  const std::string &getName() const;
  const std::string &getPluralName() const;
  IShort getAc() const;
  IShort getCombatAction(IShort round) const;
  int getGender() const;
  const BTDice &getGold() const;
  const BTDice &getHp() const;
  IShort getLevel() const;
  IShort getMagicResistance() const;
  IShort getMaxAppearing() const;
  const BTDice &getMeleeDamage() const;
  IShort getMeleeExtra() const;
  const char *getMeleeMessage() const;
  IShort getMove() const;
  IShort getPicture() const;
  IShort getRange() const;
  const BTDice &getRangedDamage() const;
  IShort getRangedExtra() const;
  const char *getRangedMessage() const;
  IShort getRangedSpell() const;
  IShort getRangedType() const;
  IShort getRateAttacks() const;
  IShort getStartDistance() const;
  unsigned int getXp() const;
  IBool isIllusion() const;
  bool isWandering() const;
  bool savingThrow(int difficulty = BTSAVE_DIFFICULTY) const;
  void setName(const std::string &nm);
  void setPluralName(const std::string &nm);
  void setLevel(IShort l);
  void setPicture(IShort pic);
  void setStartDistance(IShort d);
  void useRangedOnGroup(BTDisplay &d, BTCombatantCollection *grp, int distance, int &activeNum);
  void write(BinaryWriteFile &f);

  virtual void serialize(ObjectSerializer* s);
  void upgrade() {}

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTMonster; }
  static void readXML(const char *filename, XMLVector<BTMonster*> &monster);
  static void writeXML(const char *filename, XMLVector<BTMonster*> &monster);

 private:
  std::string name;
  std::string pluralName;
  int gender;
  IShort level;
  IShort startDistance;
  IShort move;
  IShort rateAttacks;
  IShort illusion;
  PictureIndex picture;
  std::vector<unsigned int> combatAction;
  IShort ac;
  IShort maxAppearing;
  BTDice hp;
  BTDice gold;
  IShort magicResistance;
  bool wandering;
  unsigned int xp;

  char *meleeMessage;
  BTDice meleeDamage;
  int meleeExtra;

  char *rangedMessage;
  int rangedType;
  int rangedSpell;
  BTDice rangedDamage;
  int rangedExtra;
  IShort range;
};

class BTMonsterListCompare : public BTSortCompare<BTMonster>
{
 public:
  int Compare(const BTMonster &a, const BTMonster &b) const;
};

#endif

