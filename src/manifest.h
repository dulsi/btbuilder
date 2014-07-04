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
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

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

class BTBonusManifest : public BTManifest
{
 public:
  BTBonusManifest() : bonus(0), level(0), maximum(0) {}
  BTBonusManifest(int t, int b) : BTManifest(t), bonus(b), level(0), maximum(0) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTBonusManifest; }

  int bonus;
  int level;
  int maximum;
};

class BTAttackManifest : public BTManifest
{
 public:
  BTAttackManifest() : range(0), effectiveRange(0), xSpecial(BTEXTRADAMAGE_NONE), level(0), maximum(0) {}
  BTAttackManifest(int r, int eR, const BTDice &d, int xS, int l) : BTManifest(BTSPELLTYPE_DAMAGE), range(r), effectiveRange(eR), xSpecial(xS), level(l), maximum(0) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackManifest; }

  int range;
  int effectiveRange;
  BTDice damage;
  int xSpecial;
  int level;
  int maximum;
};

class BTCureStatusManifest : public BTManifest
{
 public:
  BTCureStatusManifest() : status(0) {}
  BTCureStatusManifest(int t, int s) : BTManifest(t), status(s) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTCureStatusManifest; }

  int status;
};

class BTHealManifest : public BTManifest
{
 public:
  BTHealManifest() : level(0), maximum(0) {}
  BTHealManifest(const BTDice &h) : BTManifest(BTSPELLTYPE_HEAL), heal(h), level(0), maximum(0) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTHealManifest; }

  BTDice heal;
  int level;
  int maximum;
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
  BTPushManifest() : distance(0) {}
  BTPushManifest(int d) : BTManifest(BTSPELLTYPE_PUSH), distance(d) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPushManifest; }

  int distance;
};

class BTRegenManaManifest : public BTManifest
{
 public:
  BTRegenManaManifest() {}
  BTRegenManaManifest(BTDice m) : BTManifest(BTSPELLTYPE_REGENMANA), mana(m) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenManaManifest; }

  BTDice mana;
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
  BTSummonManifest() : monster(-1) {}
  BTSummonManifest(int t, int m) : BTManifest(t), monster(m) {}

  virtual BTManifest *clone();
  std::string createString();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSummonManifest; }

  int monster;
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

class BTDispellIllusionManifest : public BTManifest
{
 public:
  BTDispellIllusionManifest() : BTManifest(BTSPELLTYPE_DISPELLILLUSION), range(0), effectiveRange(0) {}
  BTDispellIllusionManifest(int r, int eR) : BTManifest(BTSPELLTYPE_DISPELLILLUSION), range(r), effectiveRange(eR) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDispellIllusionManifest; }

  int range;
  int effectiveRange;
};

class BTDispellMagicManifest : public BTManifest
{
 public:
  BTDispellMagicManifest() : BTManifest(BTSPELLTYPE_DISPELLMAGIC), range(0), effectiveRange(0) {}
  BTDispellMagicManifest(int r, int eR) : BTManifest(BTSPELLTYPE_DISPELLMAGIC), range(r), effectiveRange(eR) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTDispellMagicManifest; }

  int range;
  int effectiveRange;
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
  BTRegenSkillManifest() : BTManifest(BTSPELLTYPE_REGENBARD), skill(-1) {}
  BTRegenSkillManifest(int sk, const BTDice &a) : BTManifest(BTSPELLTYPE_REGENBARD), skill(sk), amount(a) {}

  virtual BTManifest *clone();
  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);
  virtual void supportOldFormat(BTDice &d, IShort &ex);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTRegenSkillManifest; }

  int skill;
  BTDice amount;
};

#endif

