/*-------------------------------------------------------------------------*\
  <song.C> -- Song implementation file

  Date      Programmer  Description
  03/14/10  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "song.h"

const char *BTSong::getName() const
{
 return name;
}

int BTSong::play(BTDisplay &d, BTPc *singer, BTCombat *combat)
{
 std::string text = singer->name;
 text += " begins to play ";
 text += name;
 text += ".";
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 int singerNum = BT_PARTYSIZE;
 for (int i = 0; i < party.size(); ++i)
 {
  if (party[i] == singer)
   singerNum = i;
 }
 unsigned int expire = 0;
 if (combat)
  expire = game->getExpiration(1);
 else
  expire = game->getExpiration(BTDice(1, 241, 239).roll());
 int musicId = d.playMusic((combat ? combatMusic : music));
 d.drawMessage(text.c_str(), game->getDelay());
 for (int i = 0; i < manifest.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = manifest[i]->manifest(d, true, combat, expire, singer->level, 9, BTTARGET_PARTY, BTTARGET_INDIVIDUAL, singerNum, musicId);
  for (std::list<BTBaseEffect*>::iterator itr = sub.begin(); itr != sub.end(); ++itr)
  {
   try
   {
    killed += (*itr)->apply(d, combat);
    game->addEffect(*itr);
   }
   catch (const BTAllResistException &e)
   {
    delete (*itr);
   }
  }
 }
 return killed;
}

void BTSong::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("music", &music);
 s->add("combatMusic", &combatMusic);
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
}

void BTSong::readXML(const char *filename, XMLVector<BTSong*> &song)
{
 XMLSerializer parser;
 parser.add("song", &song, &BTSong::create);
 parser.parse(filename, true);
}

