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

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTManifest; }

  int type;
};

class BTArmorBonusManifest : public BTManifest
{
 public:
  BTArmorBonusManifest() : bonus(0), level(0), maximum(0) {}

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTArmorBonusManifest; }

  int bonus;
  int level;
  int maximum;
};

class BTAttackManifest : public BTManifest
{
 public:
  BTAttackManifest() : range(0), effectiveRange(0), status(BTSTATUS_NONE), level(0), maximum(0) {}

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackManifest; }

  int range;
  int effectiveRange;
  BTDice damage;
  int status;
  int level;
  int maximum;
};

class BTAttackRateBonusManifest : public BTManifest
{
 public:
  BTAttackRateBonusManifest() : bonus(0), level(0), maximum(0) {}

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTAttackRateBonusManifest; }

  int bonus;
  int level;
  int maximum;
};

class BTHealManifest : public BTManifest
{
 public:
  BTHealManifest() {}

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTHealManifest; }

  BTDice heal;
  int level;
  int maximum;
};

class BTMultiManifest : public BTManifest
{
 public:
  BTMultiManifest() : restriction(BTRESTRICTION_NONE), targetOverride(BTTARGETOVERRIDE_NONE) {}

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

  virtual std::list<BTBaseEffect*> manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId);
  virtual void serialize(ObjectSerializer *s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTPushManifest; }

  int distance;
};

#endif

