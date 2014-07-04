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

void BTManifest::supportOldFormat(BTDice &d, IShort &ex)
{
}

void BTManifest::serializeSetup(ObjectSerializer *s, XMLVector<BTManifest*> &manifest)
{
 s->add("manifest", typeid(BTManifest).name(), &manifest, &BTManifest::create);
 s->add("targetedManifest", typeid(BTTargetedManifest).name(), &manifest, &BTTargetedManifest::create);
 s->add("armorBonusManifest", typeid(BTArmorBonusManifest).name(), &manifest, &BTArmorBonusManifest::create);
 s->add("attackManifest", typeid(BTAttackManifest).name(), &manifest, &BTAttackManifest::create);
 s->add("attackRateBonusManifest", typeid(BTAttackRateBonusManifest).name(), &manifest, &BTAttackRateBonusManifest::create);
 s->add("cureStatusManifest", typeid(BTCureStatusManifest).name(), &manifest, &BTCureStatusManifest::create);
 s->add("healManifest", typeid(BTHealManifest).name(), &manifest, &BTHealManifest::create);
 s->add("multiManifest", typeid(BTMultiManifest).name(), &manifest, &BTMultiManifest::create);
 s->add("pushManifest", typeid(BTPushManifest).name(), &manifest, &BTPushManifest::create);
 s->add("regenManaManifest", typeid(BTRegenManaManifest).name(), &manifest, &BTRegenManaManifest::create);
 s->add("saveBonusManifest", typeid(BTSaveBonusManifest).name(), &manifest, &BTSaveBonusManifest::create);
 s->add("hitBonusManifest", typeid(BTHitBonusManifest).name(), &manifest, &BTHitBonusManifest::create);
 s->add("scrySightManifest", typeid(BTScrySightManifest).name(), &manifest, &BTScrySightManifest::create);
 s->add("summonManifest", typeid(BTSummonManifest).name(), &manifest, &BTSummonManifest::create);
 s->add("resurrectManifest", typeid(BTResurrectManifest).name(), &manifest, &BTResurrectManifest::create);
 s->add("phaseDoorManifest", typeid(BTPhaseDoorManifest).name(), &manifest, &BTPhaseDoorManifest::create);
 s->add("dispellIllusionManifest", typeid(BTDispellIllusionManifest).name(), &manifest, &BTDispellIllusionManifest::create);
 s->add("dispellMagicManifest", typeid(BTDispellMagicManifest).name(), &manifest, &BTDispellMagicManifest::create);
 s->add("spellBindManifest", typeid(BTSpellBindManifest).name(), &manifest, &BTSpellBindManifest::create);
 s->add("regenSkillManifest", typeid(BTRegenSkillManifest).name(), &manifest, &BTRegenSkillManifest::create);
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

BTManifest *BTArmorBonusManifest::clone()
{
 return new BTArmorBonusManifest(*this);
}

std::list<BTBaseEffect*> BTArmorBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 int value = bonus;
 if (level > 0)
  value *= (casterLevel / level);
 if ((0 != maximum) && (value > maximum))
  value = maximum;
 effect.push_back(new BTArmorBonusEffect(type, expire, singer, musicId, group, target, value));
 return effect;
}

void BTArmorBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTArmorBonusManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = bonus;
 if (level > 0)
  throw FileException("Armor bonus based on caster level not supported in older file format.");
 if (maximum > 0)
  throw FileException("Armor bonus maximum not supported in older file format.");
}

BTManifest *BTAttackManifest::clone()
{
 return new BTAttackManifest(*this);
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
 BTManifest::serialize(s);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("damage", &damage);
 s->add("xSpecial", &xSpecial);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTAttackManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 d = damage;
}

BTManifest *BTAttackRateBonusManifest::clone()
{
 return new BTAttackRateBonusManifest(*this);
}

std::list<BTBaseEffect*> BTAttackRateBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 int value = bonus;
 if (level > 0)
  value *= (casterLevel / level);
 if ((0 != maximum) && (value > maximum))
  value = maximum;
 effect.push_back(new BTAttackRateBonusEffect(type, expire, singer, musicId, group, target, value));
 return effect;
}

void BTAttackRateBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTAttackRateBonusManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = bonus;
 if (level > 0)
  throw FileException("Attack rate bonus based on caster level not supported in older file format.");
 if (maximum > 0)
  throw FileException("Attack rate bonus maximum not supported in older file format.");
}

BTManifest *BTCureStatusManifest::clone()
{
 return new BTCureStatusManifest(*this);
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
 s->add("status", &status, NULL, &BTStatusLookup::lookup);
}

BTManifest *BTHealManifest::clone()
{
 return new BTHealManifest(*this);
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

void BTHealManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 d = heal;
}

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

std::list<BTBaseEffect*> BTPushManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTPushEffect(type, expire, singer, musicId, group, target, distance));
 return effect;
}

void BTPushManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("distance", &distance);
}

void BTPushManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = distance;
}

BTManifest *BTRegenManaManifest::clone()
{
 return new BTRegenManaManifest(*this);
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

void BTRegenManaManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 d = mana;
}

BTManifest *BTSaveBonusManifest::clone()
{
 return new BTSaveBonusManifest(*this);
}

std::list<BTBaseEffect*> BTSaveBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 int value = bonus;
 if (level > 0)
  value *= (casterLevel / level);
 if ((0 != maximum) && (value > maximum))
  value = maximum;
 effect.push_back(new BTSaveBonusEffect(type, expire, singer, musicId, group, target, value));
 return effect;
}

void BTSaveBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTSaveBonusManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = bonus;
 if (level > 0)
  throw FileException("Save bonus based on caster level not supported in older file format.");
 if (maximum > 0)
  throw FileException("Save bonus maximum not supported in older file format.");
}

BTManifest *BTHitBonusManifest::clone()
{
 return new BTHitBonusManifest(*this);
}

std::list<BTBaseEffect*> BTHitBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 int value = bonus;
 if (level > 0)
  value *= (casterLevel / level);
 if ((0 != maximum) && (value > maximum))
  value = maximum;
 effect.push_back(new BTHitBonusEffect(type, expire, singer, musicId, group, target, value));
 return effect;
}

void BTHitBonusManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("bonus", &bonus);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

void BTHitBonusManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = bonus;
 if (level > 0)
  throw FileException("Hit bonus based on caster level not supported in older file format.");
 if (maximum > 0)
  throw FileException("Hit bonus maximum not supported in older file format.");
}

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
}

void BTSummonManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 ex = monster;
}

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

BTManifest *BTDispellIllusionManifest::clone()
{
 return new BTDispellIllusionManifest(*this);
}

std::list<BTBaseEffect*> BTDispellIllusionManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTDispellIllusionEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, effectiveRange, distance, group, target));
 return effect;
}

void BTDispellIllusionManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
}

BTManifest *BTDispellMagicManifest::clone()
{
 return new BTDispellMagicManifest(*this);
}

std::list<BTBaseEffect*> BTDispellMagicManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTDispellMagicEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, effectiveRange, distance, group, target));
 return effect;
}

void BTDispellMagicManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
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

std::list<BTBaseEffect*> BTRegenSkillManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTRegenSkillEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, skill, amount));
 return effect;
}

void BTRegenSkillManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("skill", &skill);
 s->add("amount", &amount);
}

void BTRegenSkillManifest::supportOldFormat(BTDice &d, IShort &ex)
{
 XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
 if ((skillList[skill]->special != BTSKILLSPECIAL_SONG) && (!skillList[skill]->limited))
  throw FileException("Regen skill only supported for bard songs in older file format.");
 ex = amount.getModifier();
}

