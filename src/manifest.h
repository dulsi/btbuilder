#ifndef __MANIFEST_H
#define __MANIFEST_H
/*-------------------------------------------------------------------------*\
  <manifest.h> -- Manifest header file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "spelleffect.h"
#include "xmlserializer.h"

class BTManifest : public XMLObject
{
 public:
  BTManifest() : type(-1) {}
  BTManifest(int t) : type(t) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTManifest; }

  static void serializeSetup(ObjectSerializer *s, XMLVector<BTManifest*> &manifest);

  int type;
};

class BTTargetedManifest : public BTManifest
{
 public:
  BTTargetedManifest() {}
  BTTargetedManifest(int t) : BTManifest(t) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTTargetedManifest; }
};

class BTRangedManifest : public BTTargetedManifest
{
 public:
  BTRangedManifest() : range(0), effectiveRange(0) {}
  BTRangedManifest(int t, int r, int eR) : BTTargetedManifest(t), range(r), effectiveRange(eR) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRangedManifest; }

  int range;
  int effectiveRange;
};

class BTBonusManifest : public BTManifest
{
 public:
  BTBonusManifest() : bonus(0), level(0), maximum(0) {}
  BTBonusManifest(int t, int b) : BTManifest(t), bonus(b), level(0), maximum(0) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTBonusManifest; }

  int bonus;
  int level;
  int maximum;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTAttackManifest : public BTRangedManifest
{
 public:
  BTAttackManifest() : xSpecial(BTEXTRADAMAGE_NONE), level(0), maximum(0) {}
  BTAttackManifest(int r, int eR, const BTDice &d, int xS, int l) : BTRangedManifest(BTSPELLTYPE_DAMAGE, r, eR), damage(d), xSpecial(xS), level(l), maximum(0) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackManifest; }

  BTDice damage;
  int xSpecial;
  int level;
  int maximum;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTCureStatusManifest : public BTManifest
{
 public:
  BTCureStatusManifest() : status(BTSTATUS_POISONED) {}
  BTCureStatusManifest(int s) : BTManifest(BTSPELLTYPE_CURESTATUS), status(s) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCureStatusManifest; }

  int status;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTHealManifest : public BTManifest
{
 public:
  BTHealManifest() : level(0), maximum(0) {}
  BTHealManifest(const BTDice &h) : BTManifest(BTSPELLTYPE_HEAL), heal(h), level(0), maximum(0) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTHealManifest; }

  BTDice heal;
  int level;
  int maximum;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTMultiManifest : public BTManifest
{
 public:
  BTMultiManifest() : restriction(BTRESTRICTION_NONE), targetOverride(BTTARGETOVERRIDE_NONE) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTMultiManifest; }

  int restriction;
  int targetOverride;
  XMLVector<BTManifest*> content;
};

class BTPushManifest : public BTManifest
{
 public:
  BTPushManifest() : strength(0) {}
  BTPushManifest(int s) : BTManifest(BTSPELLTYPE_PUSH), strength(s) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPushManifest; }

  int strength;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTRegenManaManifest : public BTManifest
{
 public:
  BTRegenManaManifest() {}
  BTRegenManaManifest(BTDice m) : BTManifest(BTSPELLTYPE_REGENMANA), mana(m) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenManaManifest; }

  BTDice mana;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTScrySightManifest : public BTManifest
{
 public:
  BTScrySightManifest() : BTManifest(BTSPELLTYPE_SCRYSIGHT) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTScrySightManifest; }
};

class BTSummonManifest : public BTManifest
{
 public:
  BTSummonManifest() : monster(0) {}
  BTSummonManifest(int t, int m) : BTManifest(t), monster(m) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSummonManifest; }

  int monster;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTResurrectManifest : public BTManifest
{
 public:
  BTResurrectManifest() : BTManifest(BTSPELLTYPE_RESURRECT) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTResurrectManifest; }
};

class BTPhaseDoorManifest : public BTManifest
{
 public:
  BTPhaseDoorManifest() : BTManifest(BTSPELLTYPE_PHASEDOOR) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPhaseDoorManifest; }
};

class BTSpellBindManifest : public BTManifest
{
 public:
  BTSpellBindManifest() : BTManifest(BTSPELLTYPE_SPELLBIND) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpellBindManifest; }
};

class BTRegenSkillManifest : public BTManifest
{
 public:
  BTRegenSkillManifest() : BTManifest(BTSPELLTYPE_REGENSKILL), skill(-1) {}
  BTRegenSkillManifest(int sk, const BTDice &a) : BTManifest(BTSPELLTYPE_REGENSKILL), skill(sk), amount(a) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenSkillManifest; }

  int skill;
  BTDice amount;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

class BTLightManifest : public BTManifest
{
 public:
  BTLightManifest() : BTManifest(BTSPELLTYPE_LIGHT), illumination(5) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual int getEditFieldNumber();
  virtual const char *getEditFieldDescription(int i);
  virtual const char *getEditField(int i);
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(IShort &t, BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTLightManifest; }

  int illumination;

 private:
  static const int entries;
  static const char *description[];
  static const char *field[];
};

#endif

