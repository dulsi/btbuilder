/*-------------------------------------------------------------------------*\
  <manifest.C> -- Manifest implementation file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "manifest.h"

std::list<BTBaseEffect*> BTManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTBaseEffect(type, expire, singer, musicId));
 return effect;
}

void BTManifest::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
}

std::list<BTBaseEffect*> BTArmorBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
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

std::list<BTBaseEffect*> BTAttackManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 BTDice value = damage;
 if (level > 0)
  value.setNumber(value.getNumber() * (casterLevel / level));
 if ((0 != maximum) && (value.getNumber() > maximum))
  value.setNumber(maximum);
 effect.push_back(new BTAttackEffect(type, expire, singer, musicId, range, effectiveRange, 0, group, target, value, status, ""));
 return effect;
}

void BTAttackManifest::serialize(ObjectSerializer* s)
{
 BTManifest::serialize(s);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("damage", &damage);
 s->add("status", &status);
 s->add("level", &level);
 s->add("maximum", &maximum);
}

std::list<BTBaseEffect*> BTAttackRateBonusManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
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

std::list<BTBaseEffect*> BTHealManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, int singer, int musicId)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
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
 s->add("manifest", &content, &BTManifest::create);
 s->add("armorBonusManifest", &content, &BTArmorBonusManifest::create);
 s->add("attackManifest", &content, &BTAttackManifest::create);
 s->add("attackRateBonusManifest", &content, &BTAttackRateBonusManifest::create);
 s->add("cureStatusManifest", &content, &BTCureStatusManifest::create);
 s->add("healManifest", &content, &BTHealManifest::create);
 s->add("multiManifest", &content, &BTMultiManifest::create);
 s->add("pushManifest", &content, &BTPushManifest::create);
 s->add("regenManaManifest", &content, &BTRegenManaManifest::create);
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

