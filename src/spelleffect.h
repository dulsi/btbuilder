#ifndef __SPELLEFFECT_H
#define __SPELLEFFECT_H
/*-------------------------------------------------------------------------*\
  <spelleffect.h> -- Spell Effect header file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "xmlserializer.h"
#include "dice.h"
#include "btconst.h"

class BTDisplay;
class BTCombat;

class BTAllResistException
{
 public:
  BTAllResistException() {}
};

class BTBaseEffect : public XMLObject
{
 public:
  BTBaseEffect(int t, int x, bool s);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int maintain(BTDisplay &d, BTCombat *combat);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void serialize(ObjectSerializer *s);

  virtual bool targets(int g, int who);
  virtual bool targetsMonsters();
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  int type;
  int expiration;
  bool first;
  bool song;
};

class BTTargetedEffect : public BTBaseEffect
{
 public:
  BTTargetedEffect(int t, int x, bool s, int g, int trgt);

  virtual bool targets(int g, int who);
  virtual bool targetsMonsters();
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  int group;
  int target;
};

class BTAttackEffect : public BTTargetedEffect
{
 public:
  BTAttackEffect(int t, int x, bool s, int rng, int erng, int d, int g, int trgt, const BTDice &dam, int sts, const char *text);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int maintain(BTDisplay &d, BTCombat *combat);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  bool checkResists(BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  void displayResists(BTDisplay &d, BTCombat *combat);
  std::string message(const char *name, const char *text, int damage, const std::string& status, const BitField &flags);

  int range;
  int effectiveRange;
  int distance;
  BitField resists;
  BTDice damage;
  int status;
  std::string statusText;
};

class BTCureStatusEffect : public BTTargetedEffect
{
 public:
  BTCureStatusEffect(int t, int x, bool s, int g, int trgt, int sts);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  int status;
};

class BTHealEffect : public BTTargetedEffect
{
 public:
  BTHealEffect(int t, int x, bool s, int g, int trgt, const BTDice& h);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  BTDice heal;
};

class BTSummonMonsterEffect : public BTTargetedEffect
{
 public:
  BTSummonMonsterEffect(int t, int x, bool s, int g, int trgt);

  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
};

class BTSummonIllusionEffect : public BTTargetedEffect
{
 public:
  BTSummonIllusionEffect(int t, int x, bool s, int g, int trgt);

  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
};

class BTArmorBonusEffect : public BTTargetedEffect
{
 public:
  BTArmorBonusEffect(int t, int x, bool s, int g, int trgt, int b);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  int bonus;
};

class BTResurrectEffect : public BTTargetedEffect
{
 public:
  BTResurrectEffect(int t, int x, bool s, int g, int trgt);

  virtual int maintain(BTDisplay &d, BTCombat *combat);
};

class BTPhaseDoorEffect : public BTBaseEffect
{
 public:
  BTPhaseDoorEffect(int t, int x, bool s, int mX, int mY, int f);

  int mapX;
  int mapY;
  int facing;
};

#endif

