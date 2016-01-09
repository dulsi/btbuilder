/*-------------------------------------------------------------------------*\
  <manifest.C> -- Manifest implementation file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "manifest.h"
#include <memory>

BTManifest *BTManifest::clone()
{
 return new BTManifest(*this);
}

std::string BTManifest::createString()
{
 return std::string("Type: ") + spellTypes[type];
}

int BTManifest::getEditFieldNumber()
{
 return 0;
}

const char *BTManifest::getEditFieldDescription(int i)
{
 return NULL;
}

const char *BTManifest::getEditField(int i)
{
 return NULL;
}

bool BTManifest::hasCombatEffect() const
{
 return true;
}

std::list<BTBaseEffect*> BTManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTBaseEffect(type, expire, source));
 return effect;
}

std::list<BTBaseEffect*> BTManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 return manifest(partySpell, combat, expire, casterLevel, distance, group, target, source);
}

void BTManifest::serialize(ObjectSerializer* s)
{
 s->add("type", &type, NULL, &spellTypeLookup);
}

void BTManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
}

void BTManifest::serializeSetup(ObjectSerializer *s, XMLVector<BTManifest*> &manifest)
{
 s->add("manifest", typeid(BTManifest).name(), &manifest, &BTManifest::create);
 s->add("targetedManifest", typeid(BTTargetedManifest).name(), &manifest, &BTTargetedManifest::create);
 s->add("rangedManifest", typeid(BTRangedManifest).name(), &manifest, &BTRangedManifest::create);
 s->add("bonusManifest", typeid(BTBonusManifest).name(), &manifest, &BTBonusManifest::create);
 s->add("attackManifest", typeid(BTAttackManifest).name(), &manifest, &BTAttackManifest::create);
 s->add("cureStatusManifest", typeid(BTCureStatusManifest).name(), &manifest, &BTCureStatusManifest::create);
 s->add("healManifest", typeid(BTHealManifest).name(), &manifest, &BTHealManifest::create);
 s->add("multiManifest", typeid(BTMultiManifest).name(), &manifest, &BTMultiManifest::create);
 s->add("pushManifest", typeid(BTPushManifest).name(), &manifest, &BTPushManifest::create);
 s->add("regenManaManifest", typeid(BTRegenManaManifest).name(), &manifest, &BTRegenManaManifest::create);
 s->add("scrySightManifest", typeid(BTScrySightManifest).name(), &manifest, &BTScrySightManifest::create);
 s->add("summonManifest", typeid(BTSummonManifest).name(), &manifest, &BTSummonManifest::create);
 s->add("resurrectManifest", typeid(BTResurrectManifest).name(), &manifest, &BTResurrectManifest::create);
 s->add("phaseDoorManifest", typeid(BTPhaseDoorManifest).name(), &manifest, &BTPhaseDoorManifest::create);
 s->add("spellBindManifest", typeid(BTSpellBindManifest).name(), &manifest, &BTSpellBindManifest::create);
 s->add("regenSkillManifest", typeid(BTRegenSkillManifest).name(), &manifest, &BTRegenSkillManifest::create);
 s->add("lightManifest", typeid(BTLightManifest).name(), &manifest, &BTLightManifest::create);
 s->add("teleportManifest", typeid(BTTeleportManifest).name(), &manifest, &BTTeleportManifest::create);
 s->add("rangeBonusManifest", typeid(BTRangeBonusManifest).name(), &manifest, &BTRangeBonusManifest::create);
 s->add("detectManifest", typeid(BTDetectManifest).name(), &manifest, &BTDetectManifest::create);
 s->add("randomManifest", typeid(BTRandomManifest).name(), &manifest, &BTRandomManifest::create);
 // Backward compatability
 s->add("armorBonusManifest", "-", &manifest, &BTBonusManifest::create);
 s->add("attackRateBonusManifest", "-", &manifest, &BTBonusManifest::create);
 s->add("saveBonusManifest", "-", &manifest, &BTBonusManifest::create);
 s->add("hitBonusManifest", "-", &manifest, &BTBonusManifest::create);
}

BTManifest *BTTargetedManifest::clone()
{
 return new BTTargetedManifest(*this);
}

std::list<BTBaseEffect*> BTTargetedManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTTargetedEffect(type, expire, source, group, target));
 return effect;
}

BTManifest *BTRangedManifest::clone()
{
 return new BTRangedManifest(*this);
}

std::list<BTBaseEffect*> BTRangedManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 switch (type)
 {
  case BTSPELLTYPE_DISPELLMAGIC:
   effect.push_back(new BTDispellMagicEffect(type, expire, source, range, effectiveRange, distance, group, target));
   break;
  case BTSPELLTYPE_DISPELLILLUSION:
   effect.push_back(new BTDispellIllusionEffect(type, expire, source, range, effectiveRange, distance, group, target));
   break;
  default:
   break;
 }
 return effect;
}

void BTRangedManifest::serialize(ObjectSerializer* s)
{
 BTTargetedManifest::serialize(s);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
}

BTManifest *BTBonusManifest::clone()
{
 return new BTBonusManifest(*this);
}

std::string BTBonusManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("   Bonus: ");
 char s[50];
 sprintf(s, "%d", bonus);
 answer += s;
 if (level == 1)
  answer += " * level";
 else if (level > 1)
 {
  sprintf(s, " * (level / %d)", level);
  answer += std::string(s);
 }
 if (maximum > 0)
 {
  sprintf(s, "[max: %d]", level, maximum);
  answer += std::string(s);
 }
 return answer;
}

int BTBonusManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTBonusManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTBonusManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTBonusManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 int value = bonus;
 if (level > 0)
  value *= (casterLevel / level);
 if ((0 != maximum) && (value > maximum))
  value = maximum;
 switch (type)
 {
  case BTSPELLTYPE_ARMORBONUS:
   effect.push_back(new BTArmorBonusEffect(type, expire, source, group, target, value));
   break;
  case BTSPELLTYPE_ATTACKRATEBONUS:
   effect.push_back(new BTAttackRateBonusEffect(type, expire, source, group, target, value));
   break;
  case BTSPELLTYPE_SAVEBONUS:
   effect.push_back(new BTSaveBonusEffect(type, expire, source, group, target, value));
   break;
  case BTSPELLTYPE_HITBONUS:
   effect.push_back(new BTHitBonusEffect(type, expire, source, group, target, value));
   break;
  default:
   break;
 }
 return effect;
}

void BTBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTBonusManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 ex = bonus;
 if (level > 0)
  throw FileException("Armor bonus based on caster level not supported in older file format.");
 if (maximum > 0)
  throw FileException("Armor bonus maximum not supported in older file format.");
}

const int BTBonusManifest::entries = 3;
const char *BTBonusManifest::description[] = {"Bonus", "Level Increment", "Maximum"};
const char *BTBonusManifest::field[] = {"bonus", "level", "maximum"};

BTManifest *BTAttackManifest::clone()
{
 return new BTAttackManifest(*this);
}

std::string BTAttackManifest::createString()
{
 std::string answer = BTManifest::createString();
 if (damage.getMax() > 0)
 {
  answer += std::string("   Damage: ") + damage.createString();
  char s[50];
  if (level == 1)
   answer += " * level";
  else if (level > 1)
  {
   sprintf(s, " * (level / %d)", level);
   answer += std::string(s);
  }
  if (maximum > 0)
  {
   sprintf(s, "[max: %d]", level, maximum);
   answer += std::string(s);
  }
 }
 if (xSpecial !=  BTEXTRADAMAGE_NONE)
 {
  answer += std::string("   Condition: ") + std::string(extraDamage[xSpecial]);
 }
 if (tagOnly !=  "")
 {
  answer += std::string("   Effects Only: ") + tagOnly;
 }
 return answer;
}

int BTAttackManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTAttackManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTAttackManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTAttackManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 BTDice value = damage;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 effect.push_back(new BTAttackEffect(type, expire, source, range, effectiveRange, 0, group, target, value, xSpecial, tagOnly));
 return effect;
}

void BTAttackManifest::serialize(ObjectSerializer* s)
{
 BTRangedManifest::serialize(s);
 s->add("damage", &damage);
 s->add("xSpecial", &xSpecial, NULL, &extraDamageLookup);
 s->add("level", &level);
 s->add("maximum", &maximum);
 s->add("tagOnly", &tagOnly);
}

void BTAttackManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 if (xSpecial == BTEXTRADAMAGE_NONE)
 {
  if (level == 0)
   t = type;
  else if (level = 1)
  {
   if (maximum != 0)
    throw FileException("Maximum damage level adjustment is not supported in the old format.");
   t = BTSPELLTYPE_DAMAGEBYLEVEL;
  }
  else
   throw FileException("Damage spells can only have variable damage based on level other than one in the old format.");
 }
 else
 {
  if (d.getMax() != 0)
   throw FileException("Damage spells cannot have additional effects in the old format.");
  switch (xSpecial)
  {
   case BTEXTRADAMAGE_CRITICALHIT:
    t = BTSPELLTYPE_KILL;
    break;
   case BTEXTRADAMAGE_POISON:
    t = BTSPELLTYPE_POISON;
    break;
   case BTEXTRADAMAGE_INSANITY:
    t = BTSPELLTYPE_CAUSEINSANITY;
    break;
   case BTEXTRADAMAGE_POSSESSION:
    t = BTSPELLTYPE_POSSESS;
    break;
   case BTEXTRADAMAGE_STONED:
    t = BTSPELLTYPE_FLESHTOSTONE;
    break;
   case BTEXTRADAMAGE_PARALYSIS:
    t = BTSPELLTYPE_PARALYZE;
    break;
   case BTEXTRADAMAGE_LEVELDRAIN:
    t = BTSPELLTYPE_DRAINLEVEL;
    break;
   case BTEXTRADAMAGE_AGED:
    t = BTSPELLTYPE_AGE;
    break;
  }
 }
 d = damage;
}

const int BTAttackManifest::entries = 5;
const char *BTAttackManifest::description[] = {"Damage", "Extra Damage", "Level Increment", "Maximum", "Effect Only"};
const char *BTAttackManifest::field[] = {"damage", "xSpecial", "level", "maximum", "tagOnly"};

BTManifest *BTCureStatusManifest::clone()
{
 return new BTCureStatusManifest(*this);
}

std::string BTCureStatusManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("  Status: ");
 answer += statusLookup.getName(status);
 return answer;
}

int BTCureStatusManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTCureStatusManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTCureStatusManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTCureStatusManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTCureStatusEffect(type, expire, source, group, target, status));
 return effect;
}

void BTCureStatusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("status", &status , NULL, &statusLookup);
}

void BTCureStatusManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 switch (status)
 {
  case BTSTATUS_LEVELDRAIN:
   t = BTSPELLTYPE_RESTORELEVELS;
   break;
  case BTSTATUS_POISONED:
   t = BTSPELLTYPE_CUREPOISON;
   break;
  case BTSTATUS_INSANE:
   t = BTSPELLTYPE_CUREINSANITY;
   break;
  case BTSTATUS_AGED:
   t = BTSPELLTYPE_YOUTH;
   break;
  case BTSTATUS_POSSESSED:
   t = BTSPELLTYPE_DISPOSSESS;
   break;
  case BTSTATUS_STONED:
   t = BTSPELLTYPE_STONETOFLESH;
   break;
  case BTSTATUS_PARALYZED:
   t = BTSPELLTYPE_CUREPARALYZE;
   break;
  default:
   throw FileException("Unsupported cure status effect.");
 }
}

const int BTCureStatusManifest::entries = 1;
const char *BTCureStatusManifest::description[] = {"Status"};
const char *BTCureStatusManifest::field[] = {"status"};

BTManifest *BTHealManifest::clone()
{
 return new BTHealManifest(*this);
}

std::string BTHealManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("  Heal: ") + heal.createString();
 char s[50];
 if (level == 1)
  answer += " * level";
 else if (level > 1)
 {
  sprintf(s, " * (level / %d)", level);
  answer += std::string(s);
 }
 if (maximum > 0)
 {
  sprintf(s, "[max: %d]", level, maximum);
  answer += std::string(s);
 }
 return answer;
}

int BTHealManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTHealManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTHealManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTHealManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 BTDice value = heal;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 effect.push_back(new BTHealEffect(type, expire, source, group, target, value));
 return effect;
}

void BTHealManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("heal", &heal);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTHealManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 d = heal;
}

const int BTHealManifest::entries = 3;
const char *BTHealManifest::description[] = {"Heal", "Level Increment", "Maximum"};
const char *BTHealManifest::field[] = {"heal", "level", "maximum"};

BTManifest *BTMultiManifest::clone()
{
 return new BTMultiManifest(*this);
}

std::list<BTBaseEffect*> BTMultiManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 if ((restriction == BTRESTRICTION_COMBAT) && (combat == NULL))
  return effect;
 if ((restriction == BTRESTRICTION_NONCOMBAT) && (combat != NULL))
  return effect;
 if (targetOverride == BTTARGETOVERRIDE_SINGER)
 {
  group = BTTARGET_PARTY;
  target = source.who;
 }
 else if (targetOverride == BTTARGETOVERRIDE_ALLMONSTERS)
 {
  group = BTTARGET_ALLMONSTERS;
  target = BTTARGET_INDIVIDUAL;
 }
 for (int i = 0; i < content.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = content[i]->manifest(partySpell, combat, expire, casterLevel, distance, group, target, source);
  for (std::list<BTBaseEffect*>::iterator itr = sub.begin(); itr != sub.end(); ++itr)
  {
   effect.push_back(*itr);
  }
 }
 return effect;
}

std::list<BTBaseEffect*> BTMultiManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 if ((restriction == BTRESTRICTION_COMBAT) && (combat == NULL))
  return effect;
 if ((restriction == BTRESTRICTION_NONCOMBAT) && (combat != NULL))
  return effect;
 if (targetOverride == BTTARGETOVERRIDE_SINGER)
 {
  group = BTTARGET_PARTY;
  target = source.who;
 }
 else if (targetOverride == BTTARGETOVERRIDE_ALLMONSTERS)
 {
  group = BTTARGET_ALLMONSTERS;
  target = BTTARGET_INDIVIDUAL;
 }
 for (int i = 0; i < content.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = content[i]->manifest(d, partySpell, combat, expire, casterLevel, distance, group, target, source);
  for (std::list<BTBaseEffect*>::iterator itr = sub.begin(); itr != sub.end(); ++itr)
  {
   effect.push_back(*itr);
  }
 }
 return effect;
}

void BTMultiManifest::serialize(ObjectSerializer* s)
{
 s->add("restriction", &restriction, NULL, &restrictionLookup);
 s->add("targetOverride", &targetOverride, NULL, &targetOverrideLookup);
 serializeSetup(s, content);
}

BTManifest *BTPushManifest::clone()
{
 return new BTPushManifest(*this);
}

std::string BTPushManifest::createString()
{
 char s[50];
 sprintf(s, "%d", strength);
 return BTManifest::createString() + std::string("   Force: ") + std::string(s);
}

std::list<BTBaseEffect*> BTPushManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTPushEffect(type, expire, source, group, target, strength));
 return effect;
}

int BTPushManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTPushManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTPushManifest::getEditField(int i)
{
 return field[i];
}

void BTPushManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("strength", &strength);
}

void BTPushManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 ex = strength;
}

const int BTPushManifest::entries = 1;
const char *BTPushManifest::description[] = {"Force"};
const char *BTPushManifest::field[] = {"strength"};

BTManifest *BTRegenManaManifest::clone()
{
 return new BTRegenManaManifest(*this);
}

std::string BTRegenManaManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("  Amount: ") + mana.createString();
 if (delay)
 {
  char s[50];
  sprintf(s, "%d", delay);
  answer += std::string(" per ") + std::string(s) + std::string(" turn");
  if (delay > 1)
   answer += "s";
 }
 return answer;
}

int BTRegenManaManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTRegenManaManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTRegenManaManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTRegenManaManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTRegenManaEffect(type, expire, source, group, target, mana, delay));
 return effect;
}

void BTRegenManaManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("mana", &mana);
 s->add("delay", &delay);
}

void BTRegenManaManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 d = mana;
}

const int BTRegenManaManifest::entries = 2;
const char *BTRegenManaManifest::description[] = {"Amount", "Delay"};
const char *BTRegenManaManifest::field[] = {"mana", "delay"};

BTManifest *BTScrySightManifest::clone()
{
 return new BTScrySightManifest(*this);
}

std::list<BTBaseEffect*> BTScrySightManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTScrySightEffect(type, expire, source));
 return effect;
}

BTManifest *BTSummonManifest::clone()
{
 return new BTSummonManifest(*this);
}

std::string BTSummonManifest::createString()
{
 BTFactory<BTMonster> &monList(BTCore::getCore()->getMonsterList());
 return BTManifest::createString() + std::string("   Name: ") + monList[monster].getName();
}

int BTSummonManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTSummonManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTSummonManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTSummonManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 if (party.size() >= BT_PARTYSIZE)
 {
  BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
  std::string text = "No room in your party. ";
  text += monsterList[monster].getName();
  text += " cannot join!";
  d.drawMessage(text.c_str(), game->getDelay());
 }
 else
 {
  BTPc *pc = new BTPc(monster, ((type == BTSPELLTYPE_SUMMONMONSTER) ? BTJOB_MONSTER : BTJOB_ILLUSION));
  party.add(d, pc);
  d.drawStats();
  if ((BTTIME_PERMANENT != expire) && (BTTIME_CONTINUOUS != expire))
  {
   if (type == BTSPELLTYPE_SUMMONMONSTER)
    effect.push_back(new BTSummonMonsterEffect(type, expire, source, BTTARGET_PARTY, party.size() - 1));
   else
    effect.push_back(new BTSummonIllusionEffect(type, expire, source, BTTARGET_PARTY, party.size() - 1));
  }
 }
 return effect;
}

void BTSummonManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("monster", &monster);
 s->add("monsterName", &monster, NULL, &BTCore::getCore()->getMonsterList());
}

void BTSummonManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 ex = monster;
}

const int BTSummonManifest::entries = 1;
const char *BTSummonManifest::description[] = {"Monster"};
const char *BTSummonManifest::field[] = {"monsterName"};

BTManifest *BTResurrectManifest::clone()
{
 return new BTResurrectManifest(*this);
}

std::list<BTBaseEffect*> BTResurrectManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTResurrectEffect(type, expire, source, group, target));
 return effect;
}

std::list<BTBaseEffect*> BTResurrectManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   effect.push_back(new BTResurrectEffect(type, expire, source, group, target));
  }
  else
  {
   if (!party[target]->isAlive())
   {
    std::string text = party[target]->name;
    text += " rises from the dead!";
    party[target]->status.clear(BTSTATUS_DEAD);
    party[target]->hp = 1;
    d.drawMessage(text.c_str(), game->getDelay());
    d.drawStats();
   }
  }
 }
 return effect;
}

BTManifest *BTPhaseDoorManifest::clone()
{
 return new BTPhaseDoorManifest(*this);
}

std::list<BTBaseEffect*> BTPhaseDoorManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 BTGame *game = BTGame::getGame();
 int x = game->getX();
 int y = game->getY();
 int f = game->getFacing();
 for (int i = 0; i < 4; ++i)
 {
  int testX = (x + (Psuedo3D::changeXY[f][0] * i) + game->getMap()->getXSize()) % game->getMap()->getXSize();
  int testY = (y + (Psuedo3D::changeXY[f][1] * i) + game->getMap()->getYSize()) % game->getMap()->getYSize();
  int str = game->testWallStrength(testX, testY, f);
  if (1 == str)
  {
   effect.push_back(new BTPhaseDoorEffect(type, BTTIME_MAP, source, testX, testY, f));
   break;
  }
  else if (2 == str)
   break;
 }
 return effect;
}

BTManifest *BTSpellBindManifest::clone()
{
 return new BTSpellBindManifest(*this);
}

std::list<BTBaseEffect*> BTSpellBindManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTSpellBindEffect(type, expire, source, group, target));
 return effect;
}

BTManifest *BTRegenSkillManifest::clone()
{
 return new BTRegenSkillManifest(*this);
}

std::string BTRegenSkillManifest::createString()
{
 XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
 std::string answer = BTManifest::createString() + std::string("   Skill: ") + skillList[skill]->name + std::string("   Amount: ") + amount.createString();
 if (unlimited)
  answer += "   Unlimited Use";
 return answer;
}

int BTRegenSkillManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTRegenSkillManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTRegenSkillManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTRegenSkillManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTRegenSkillEffect(type, expire, source, group, target, skill, amount, unlimited));
 return effect;
}

void BTRegenSkillManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("skill", &skill, NULL, &BTCore::getCore()->getSkillList());
 s->add("amount", &amount);
 s->add("unlimited", &unlimited);
}

void BTRegenSkillManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
 if ((skillList[skill]->special != BTSKILLSPECIAL_SONG) && (!skillList[skill]->limited))
  throw FileException("Regen skill only supported for bard songs in older file format.");
 if (amount.getMin() != amount.getMax())
  throw FileException("Regen skill does not support variable amount in older file format.");
 if (unlimited)
  throw FileException("Regen skill does not support unlimited use in older file format.");
 t = BTSPELLTYPE_REGENBARD;
 ex = amount.getModifier();
}

const int BTRegenSkillManifest::entries = 3;
const char *BTRegenSkillManifest::description[] = {"Skill", "Amount", "Unlimited Use"};
const char *BTRegenSkillManifest::field[] = {"skill", "amount", "unlimited"};

BTManifest *BTLightManifest::clone()
{
 return new BTLightManifest(*this);
}

std::string BTLightManifest::createString()
{
 char s[50];
 std::string answer = BTManifest::createString() + std::string("  Illumination: ");
 sprintf(s, "%d", illumination);
 answer += std::string(s);
 return answer;
}

int BTLightManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTLightManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTLightManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTLightManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTLightEffect(type, expire, source, group, target, illumination));
 return effect;
}

void BTLightManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("illumination", &illumination);
}

void BTLightManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 if (illumination != 5)
  throw FileException("Unsupported illumination level.");
}

const int BTLightManifest::entries = 1;
const char *BTLightManifest::description[] = {"Illumination"};
const char *BTLightManifest::field[] = {"illumination"};

BTManifest *BTTeleportManifest::clone()
{
 return new BTTeleportManifest(*this);
}

bool BTTeleportManifest::hasCombatEffect() const
{
 return false;
}

std::list<BTBaseEffect*> BTTeleportManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 if (combat != NULL)
  return effect;
 d.clearText();
 d.addText("Teleport");
 BTDisplay::selectItem items[3];
 items[0].name = "North: ";
 items[0].flags.set(BTSELECTFLAG_NUMBER);
 items[0].flags.set(BTSELECTFLAG_SHOWVALUE);
 items[1].name = "East: ";
 items[1].flags.set(BTSELECTFLAG_NUMBER);
 items[1].flags.set(BTSELECTFLAG_SHOWVALUE);
 items[2].name = "Down: ";
 items[2].flags.set(BTSELECTFLAG_NUMBER);
 items[2].flags.set(BTSELECTFLAG_SHOWVALUE);
 int start = 0;
 int select = 0;
 d.addSelection(items, 3, start, select);
 int key = d.process();
 d.clearText();
 if (key != 27)
 {
  BTGame *game = BTGame::getGame();
  std::unique_ptr<BTMap> autoMap;
  BTMap *map = game->getMap();
  if (items[2].value != 0)
  {
   std::string newFile = game->descendMap(items[2].value);
   if (newFile == "")
    return effect;
   map = game->readMap(newFile);
   std::unique_ptr<BTMap> tmpMap(map);
   autoMap = std::move(tmpMap);
  }
  int x = game->getX() + items[1].value;
  while (x < 0)
   x += map->getXSize();
  x = x % map->getXSize();
  int y = game->getY() - items[0].value;
  while (y < 0)
   y += map->getYSize();
  y = y % map->getYSize();
  int sp = map->getSquare(x, y).getSpecial();
  if (sp == -1)
  {
   if (map->getFlag().isSet(BTSPECIALFLAG_ANTITELEPORT))
    return effect;
  }
  else
  {
   if (map->getSpecial(sp)->getFlag().isSet(BTSPECIALFLAG_ANTITELEPORT))
    return effect;
  }
  effect.push_back(new BTTeleportEffect(type, expire, source, x, y, map->getFilename()));
 }
 return effect;
}

BTManifest *BTRangeBonusManifest::clone()
{
 return new BTRangeBonusManifest(*this);
}

std::string BTRangeBonusManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("   Bonus: ") + bonus.createString();
 char s[50];
 if (level == 1)
  answer += " * level";
 else if (level > 1)
 {
  sprintf(s, " * (level / %d)", level);
  answer += std::string(s);
 }
 if (maximum > 0)
 {
  sprintf(s, "[max: %d]", level, maximum);
  answer += std::string(s);
 }
 return answer;
}

int BTRangeBonusManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTRangeBonusManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTRangeBonusManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTRangeBonusManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 BTDice value = bonus;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 switch (type)
 {
  case BTSPELLTYPE_DAMAGEBONUS:
   effect.push_back(new BTDamageBonusEffect(type, expire, source, group, target, value, true));
   break;
  default:
   break;
 }
 return effect;
}

void BTRangeBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

const int BTRangeBonusManifest::entries = 3;
const char *BTRangeBonusManifest::description[] = {"Bonus", "Level Increment", "Maximum"};
const char *BTRangeBonusManifest::field[] = {"bonus", "level", "maximum"};

BTManifest *BTDetectManifest::clone()
{
 return new BTDetectManifest(*this);
}

std::string BTDetectManifest::createString()
{
 BTCore *game = BTCore::getCore();
 BTSpecialFlagList &flagList = game->getSpecialFlagList();
 char s[50];
 std::string answer = BTManifest::createString() + std::string("   Range: ");
 sprintf(s, "%d", range);
 answer += std::string(s);
 answer += "  Flags: ";
 answer += flags.print(&flagList, false);
 return answer;
}

int BTDetectManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTDetectManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTDetectManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTDetectManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTDetectEffect(type, expire, source, range, flags));
 return effect;
}

void BTDetectManifest::serialize(ObjectSerializer* s)
{
 BTCore *game = BTCore::getCore();
 BTSpecialFlagList &flagList = game->getSpecialFlagList();
 BTManifest::serialize(s);
 s->add("range", &range);
 s->add("flag", &flags, &flagList);
}

const int BTDetectManifest::entries = 2;
const char *BTDetectManifest::description[] = {"Range", "Flags"};
const char *BTDetectManifest::field[] = {"range", "flag"};

BTRandomManifestChance::BTRandomManifestChance(const BTRandomManifestChance &other)
: chance(other.chance)
{
 for (int i = 0; i < other.content.size(); i++)
 {
  content.push_back(other.content[i]->clone());
 }
}

void BTRandomManifestChance::serialize(ObjectSerializer *s)
{
 s->add("chance", &chance);
 BTManifest::serializeSetup(s, content);
}

BTRandomManifest::BTRandomManifest(const BTRandomManifest &other)
: roll(other.roll)
{
 for (int i = 0; i < other.chance.size(); i++)
 {
  chance.push_back(new BTRandomManifestChance(*other.chance[i]));
 }
}

BTManifest *BTRandomManifest::clone()
{
 return new BTRandomManifest(*this);
}

std::string BTRandomManifest::createString()
{
 std::string answer = BTManifest::createString() + std::string("   Roll: ");
 answer += roll.createString();
 return answer;
}

int BTRandomManifest::getEditFieldNumber()
{
 return entries;
}

const char *BTRandomManifest::getEditFieldDescription(int i)
{
 return description[i];
}

const char *BTRandomManifest::getEditField(int i)
{
 return field[i];
}

std::list<BTBaseEffect*> BTRandomManifest::manifest(bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, const BTEffectSource &source)
{
 std::list<BTBaseEffect*> effect;
 int result = roll.roll();
 for (int i = 0; i < chance.size(); i++)
 {
  if (result <= chance[i]->chance)
  {
   for (int k = 0; k < chance[i]->content.size(); ++k)
   {
    std::list<BTBaseEffect*> sub = chance[i]->content[k]->manifest(partySpell, combat, expire, casterLevel, distance, group, target, source);
    for (std::list<BTBaseEffect*>::iterator itr = sub.begin(); itr != sub.end(); ++itr)
    {
     effect.push_back(*itr);
    }
   }
   break;
  }
  else
   result -= chance[i]->chance;
 }
 return effect;
}

void BTRandomManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("roll", &roll);
 s->add("chance", &chance, &BTRandomManifestChance::create);
}

const int BTRandomManifest::entries = 1;
const char *BTRandomManifest::description[] = {"Roll"};
const char *BTRandomManifest::field[] = {"roll"};
