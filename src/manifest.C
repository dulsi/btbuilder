/*-------------------------------------------------------------------------*\
  <manifest.C> -- Manifest implementation file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "manifest.h"

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

std::list<BTBaseEffect*> BTManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTBaseEffect(type, expire, singer, musicId));
 return effect;
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

std::list<BTBaseEffect*> BTTargetedManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTTargetedEffect(type, expire, singer, musicId, group, target));
 return effect;
}

BTManifest *BTRangedManifest::clone()
{
 return new BTRangedManifest(*this);
}

std::list<BTBaseEffect*> BTRangedManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 switch (type)
 {
  case BTSPELLTYPE_DISPELLMAGIC:
   effect.push_back(new BTDispellMagicEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, effectiveRange, distance, group, target));
   break;
  case BTSPELLTYPE_DISPELLILLUSION:
   effect.push_back(new BTDispellIllusionEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, effectiveRange, distance, group, target));
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

std::list<BTBaseEffect*> BTBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
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
   effect.push_back(new BTArmorBonusEffect(type, expire, singer, musicId, group, target, value));
   break;
  case BTSPELLTYPE_ATTACKRATEBONUS:
   effect.push_back(new BTAttackRateBonusEffect(type, expire, singer, musicId, group, target, value));
   break;
  case BTSPELLTYPE_SAVEBONUS:
   effect.push_back(new BTSaveBonusEffect(type, expire, singer, musicId, group, target, value));
   break;
  case BTSPELLTYPE_HITBONUS:
   effect.push_back(new BTHitBonusEffect(type, expire, singer, musicId, group, target, value));
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

std::list<BTBaseEffect*> BTAttackManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 BTDice value = damage;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 effect.push_back(new BTAttackEffect(type, expire, singer, musicId, range, effectiveRange, 0, group, target, value, xSpecial));
 return effect;
}

void BTAttackManifest::serialize(ObjectSerializer* s)
{
 BTRangedManifest::serialize(s);
 s->add("damage", &damage);
 s->add("xSpecial", &xSpecial, NULL, &extraDamageLookup);
 s->add("level", &level);
 s->add("maximum", &maximum);
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

const int BTAttackManifest::entries = 4;
const char *BTAttackManifest::description[] = {"Damage", "Extra Damage", "Level Increment", "Maximum"};
const char *BTAttackManifest::field[] = {"damage", "xSpecial", "level", "maximum"};

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

std::list<BTBaseEffect*> BTCureStatusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTCureStatusEffect(type, expire, singer, musicId, group, target, status));
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

std::list<BTBaseEffect*> BTHealManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 BTDice value = heal;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 effect.push_back(new BTHealEffect(type, expire, singer, musicId, group, target, value));
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

std::list<BTBaseEffect*> BTMultiManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 if ((restriction == BTRESTRICTION_COMBAT) && (combat == NULL))
  return effect;
 if ((restriction == BTRESTRICTION_NONCOMBAT) && (combat != NULL))
  return effect;
 if (targetOverride == BTTARGETOVERRIDE_SINGER)
 {
  group = BTTARGET_PARTY;
  target = singer;
 }
 else if (targetOverride == BTTARGETOVERRIDE_ALLMONSTERS)
 {
  group = BTTARGET_ALLMONSTERS;
  target = BTTARGET_INDIVIDUAL;
 }
 for (int i = 0; i < content.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = content[i]->manifest(d, partySpell, combat, expire, casterLevel, distance, group, target, singer, musicId);
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

std::list<BTBaseEffect*> BTPushManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTPushEffect(type, expire, singer, musicId, group, target, strength));
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
 return BTManifest::createString() + std::string("  Amount: ") + mana.createString();
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

std::list<BTBaseEffect*> BTRegenManaManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTRegenManaEffect(type, expire, singer, musicId, group, target, mana));
 return effect;
}

void BTRegenManaManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("mana", &mana);
}

void BTRegenManaManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 t = type;
 d = mana;
}

const int BTRegenManaManifest::entries = 1;
const char *BTRegenManaManifest::description[] = {"Amount"};
const char *BTRegenManaManifest::field[] = {"mana"};

BTManifest *BTScrySightManifest::clone()
{
 return new BTScrySightManifest(*this);
}

std::list<BTBaseEffect*> BTScrySightManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTScrySightEffect(type, expire, singer, musicId));
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

std::list<BTBaseEffect*> BTSummonManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
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
    effect.push_back(new BTSummonMonsterEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, BTTARGET_PARTY, party.size() - 1));
   else
    effect.push_back(new BTSummonIllusionEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, BTTARGET_PARTY, party.size() - 1));
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

std::list<BTBaseEffect*> BTResurrectManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 if (BTTARGET_PARTY == group)
 {
  if (BTTARGET_INDIVIDUAL == target)
  {
   effect.push_back(new BTResurrectEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target));
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

std::list<BTBaseEffect*> BTPhaseDoorManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
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
   effect.push_back(new BTPhaseDoorEffect(type, BTTIME_MAP, BTTARGET_NOSINGER, BTMUSICID_NONE, testX, testY, f));
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

std::list<BTBaseEffect*> BTSpellBindManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTSpellBindEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target));
 return effect;
}

BTManifest *BTRegenSkillManifest::clone()
{
 return new BTRegenSkillManifest(*this);
}

std::string BTRegenSkillManifest::createString()
{
 XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
 return BTManifest::createString() + std::string("   Skill: ") + skillList[skill]->name + std::string("   Amount: ") + amount.createString();
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

std::list<BTBaseEffect*> BTRegenSkillManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTRegenSkillEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, skill, amount));
 return effect;
}

void BTRegenSkillManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("skill", &skill, NULL, &BTCore::getCore()->getSkillList());
 s->add("amount", &amount);
}

void BTRegenSkillManifest::supportOldFormat(IShort &t, BTDice &d, IShort &ex)
{
 XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
 if ((skillList[skill]->special != BTSKILLSPECIAL_SONG) && (!skillList[skill]->limited))
  throw FileException("Regen skill only supported for bard songs in older file format.");
 if (amount.getMin() != amount.getMax())
  throw FileException("Regen skill does not support variable amount in older file format.");
 t = BTSPELLTYPE_REGENBARD;
 ex = amount.getModifier();
}

const int BTRegenSkillManifest::entries = 2;
const char *BTRegenSkillManifest::description[] = {"Skill", "Amount"};
const char *BTRegenSkillManifest::field[] = {"skill", "amount"};

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

std::list<BTBaseEffect*> BTLightManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTLightEffect(type, expire, singer, musicId, group, target, illumination));
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

