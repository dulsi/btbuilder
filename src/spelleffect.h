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
class BTCombatantCollection;
class BTCombatant;
class BTGame;

class BTDamageBonus : public XMLObject
{
 public:
  BTDamageBonus(const BTDice &amt, bool m) : amount(amt), melee(m) {}
  BTDamageBonus() {}

  bool operator==(const BTDamageBonus &val) const;

  int apply(bool m);
  int getMax() { return amount.getMax(); }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDamageBonus; }

 private:
  BTDice amount;
  bool melee;
};

class BTAllResistException
{
 public:
  BTAllResistException() {}
};

class BTEffectSource : public XMLObject
{
 public:
  BTEffectSource(unsigned int t, int w, int e);
  BTEffectSource();

  virtual void serialize(ObjectSerializer *s);

  unsigned int type;
  int who;
  int effectID;
};

class BTBaseEffect : public XMLObject
{
 public:
  BTBaseEffect(int t, int x, const BTEffectSource &s);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int maintain(BTDisplay &d, BTCombat *combat);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void serialize(ObjectSerializer *s);

  virtual bool targets(int g, int who, bool exact = true);
  virtual bool targetsMonsters();
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  bool isExpired(BTGame *g);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTBaseEffect(0, 0, BTEffectSource()); }

  int type;
  int expiration;
  bool expire;
  bool first;
  BTEffectSource source;
};

class BTTargetedEffect : public BTBaseEffect
{
 public:
  BTTargetedEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual void serialize(ObjectSerializer *s);

  virtual bool targets(int g, int who, bool exact = true);
  virtual bool targetsMonsters();
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTTargetedEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }

  int group;
  int target;
};

class BTResistedEffect : public BTTargetedEffect
{
 public:
  BTResistedEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual void serialize(ObjectSerializer *s);

  bool checkResists(BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTResistedEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }

  BitField resists;
};

class BTAttackEffect : public BTResistedEffect
{
 public:
  BTAttackEffect(int t, int x, const BTEffectSource &s, int rng, int erng, int d, int g, int trgt, const BTDice &dam, int sts, const std::string& tOnly);

  virtual void serialize(ObjectSerializer *s);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int maintain(BTDisplay &d, BTCombat *combat);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void move(int g, int who, int where);
  virtual void remove(BTCombat *combat, int g, int who);

  void displayResists(BTDisplay &d, BTCombat *combat);
  int applyToGroup(BTDisplay &d, BTCombatantCollection *grp, int resistOffset = 0);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackEffect(0, 0, BTEffectSource(), 0, 0, 0, BTTARGET_NONE, BTTARGET_NONE, BTDice(), BTEXTRADAMAGE_NONE, ""); }

  int range;
  int effectiveRange;
  int distance;
  BTDice damage;
  int status;
  std::string tagOnly;
};

class BTCureStatusEffect : public BTTargetedEffect
{
 public:
  BTCureStatusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int sts);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCureStatusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, BTSTATUS_NONE); }

  int status;
};

class BTHealEffect : public BTTargetedEffect
{
 public:
  BTHealEffect(int t, int x, const BTEffectSource &s, int g, int trgt, const BTDice& h);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTHealEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, BTDice()); }

  BTDice heal;
};

class BTSummonMonsterEffect : public BTTargetedEffect
{
 public:
  BTSummonMonsterEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSummonMonsterEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }
};

class BTSummonIllusionEffect : public BTTargetedEffect
{
 public:
  BTSummonIllusionEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSummonIllusionEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }
};

class BTDispellIllusionEffect : public BTTargetedEffect
{
 public:
  BTDispellIllusionEffect(int t, int x, const BTEffectSource &s, int rng, int erng, int d, int g, int trgt);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  int applyToGroup(BTDisplay &d, BTCombatantCollection *grp);
  int apply(BTDisplay &d, BTCombatant *target);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDispellIllusionEffect(0, 0, BTEffectSource(), 0, 0, 0, BTTARGET_NONE, BTTARGET_NONE); }

  int range;
  int effectiveRange;
  int distance;
};

class BTNonStackingBonusEffect : public BTTargetedEffect
{
 public:
  BTNonStackingBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual void serialize(ObjectSerializer *s);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt) = 0;
  virtual bool greater(BTNonStackingBonusEffect *b) = 0;
  virtual bool isGood() = 0;
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt) = 0;

  bool active;
};

class BTArmorBonusEffect : public BTNonStackingBonusEffect
{
 public:
  BTArmorBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int b);

  virtual void serialize(ObjectSerializer *s);

  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);
  virtual bool greater(BTNonStackingBonusEffect *b);
  virtual bool isGood();
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTArmorBonusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0); }

  int bonus;
};

class BTHitBonusEffect : public BTNonStackingBonusEffect
{
 public:
  BTHitBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int b);

  virtual void serialize(ObjectSerializer *s);

  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);
  virtual bool greater(BTNonStackingBonusEffect *b);
  virtual bool isGood();
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTHitBonusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0); }

  int bonus;
};

class BTResurrectEffect : public BTTargetedEffect
{
 public:
  BTResurrectEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTResurrectEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }
};

class BTDispellMagicEffect : public BTTargetedEffect
{
 public:
  BTDispellMagicEffect(int t, int x, const BTEffectSource &s, int rng, int erng, int d, int g, int trgt);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDispellMagicEffect(0, 0, BTEffectSource(), 0, 0, 0, BTTARGET_NONE, BTTARGET_NONE); }

  int range;
  int effectiveRange;
  int distance;
};

class BTPhaseDoorEffect : public BTBaseEffect
{
 public:
  BTPhaseDoorEffect(int t, int x, const BTEffectSource &s, int mX, int mY, int f);

  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPhaseDoorEffect(0, 0, BTEffectSource(), 0, 0, 0); }

  int mapX;
  int mapY;
  int facing;
};

class BTRegenSkillEffect : public BTTargetedEffect
{
 public:
  BTRegenSkillEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int sk, const BTDice& u, bool unl);

  virtual void serialize(ObjectSerializer *s);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual int maintain(BTDisplay &d, BTCombat *combat);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenSkillEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0, BTDice(), false); }

  int skill;
  BTDice use;
  bool unlimited;
};

class BTPushEffect : public BTTargetedEffect
{
 public:
  BTPushEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int dis);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPushEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0); }

  int distance;
};

class BTAttackRateBonusEffect : public BTNonStackingBonusEffect
{
 public:
  BTAttackRateBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int b);

  virtual void serialize(ObjectSerializer *s);

  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);
  virtual bool greater(BTNonStackingBonusEffect *b);
  virtual bool isGood();
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackRateBonusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0); }

  int bonus;
};

class BTRegenManaEffect : public BTTargetedEffect
{
 public:
  BTRegenManaEffect(int t, int x, const BTEffectSource &s, int g, int trgt, const BTDice& sp, int d);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenManaEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, BTDice(), 0); }

  BTDice mana;
  int delay;
};

class BTSaveBonusEffect : public BTNonStackingBonusEffect
{
 public:
  BTSaveBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int b);

  virtual void serialize(ObjectSerializer *s);

  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);
  virtual bool greater(BTNonStackingBonusEffect *b);
  virtual bool isGood();
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSaveBonusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 0); }

  int bonus;
};

class BTScrySightEffect : public BTBaseEffect
{
 public:
  BTScrySightEffect(int t, int x, const BTEffectSource &s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTScrySightEffect(0, 0, BTEffectSource()); }
};

class BTSpellBindEffect : public BTResistedEffect
{
 public:
  BTSpellBindEffect(int t, int x, const BTEffectSource &s, int g, int trgt);

  virtual int apply(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);
  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpellBindEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE); }
};

class BTLightEffect : public BTTargetedEffect
{
 public:
  BTLightEffect(int t, int x, const BTEffectSource &s, int g, int trgt, int illum);

  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLightEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, 5); }

  int illumination;
};

class BTTeleportEffect : public BTBaseEffect
{
 public:
  BTTeleportEffect(int t, int x, const BTEffectSource &s, int mX, int mY, const std::string &mFile);

  virtual void finish(BTDisplay &d, BTCombat *combat, int g = BTTARGET_NONE, int trgt = BTTARGET_INDIVIDUAL);

  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTTeleportEffect(0, 0, BTEffectSource(), 0, 0, ""); }

  int mapX;
  int mapY;
  std::string mapFile;
};

class BTDamageBonusEffect : public BTNonStackingBonusEffect
{
 public:
  BTDamageBonusEffect(int t, int x, const BTEffectSource &s, int g, int trgt, const BTDice &d, bool melee);

  virtual void serialize(ObjectSerializer *s);

  virtual int applyBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);
  virtual bool greater(BTNonStackingBonusEffect *b);
  virtual bool isGood();
  virtual void finishBonus(BTDisplay &d, BTCombat *combat, int g, int trgt);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDamageBonusEffect(0, 0, BTEffectSource(), BTTARGET_NONE, BTTARGET_NONE, BTDice(0, 2), true); }

  BTDamageBonus bonus;
};

class BTDetectEffect : public BTBaseEffect
{
 public:
  BTDetectEffect(int t, int x, const BTEffectSource &s, int r, const BitField &f);

  virtual void serialize(ObjectSerializer *s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDetectEffect(0, 0, BTEffectSource(), 0, BitField()); }

  int range;
  BitField flags;
};

class BTLocationEffect : public BTBaseEffect
{
 public:
  BTLocationEffect(int t, int x, const BTEffectSource &s);

  virtual int maintain(BTDisplay &d, BTCombat *combat);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLocationEffect(0, 0, BTEffectSource()); }
};

#endif

