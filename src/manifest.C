/*-------------------------------------------------------------------------*\
  <manifest.C> -- Manifest implementation file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "manifest.h"

std::list<BTBaseEffect*> BTManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, bool song)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 std::list<BTBaseEffect*> effect;
 effect.push_back(new BTBaseEffect(type, expire, song));
 return effect;
}

void BTManifest::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
}

std::list<BTBaseEffect*> BTMultiManifest::manifest(BTDisplay &d, bool partySpell, BTCombat *combat, unsigned int expire, int casterLevel, int distance, int group, int target, bool song)
{
 std::list<BTBaseEffect*> effect;
 if ((restriction == BTRESTRICTION_COMBAT) && (combat == NULL))
  return effect;
 if ((restriction == BTRESTRICTION_NONCOMBAT) && (combat != NULL))
  return effect;
 for (int i = 0; i < content.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = content[i]->manifest(d, partySpell, combat, expire, casterLevel, distance, group, target, song);
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
 s->add("manifest", &content, &BTManifest::create);
 s->add("multiManifest", &content, &BTMultiManifest::create);
}

