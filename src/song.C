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

int BTSong::play(BTDisplay &d, const char *caster, BTCombat *combat, int casterLevel)
{
 std::string text = caster;
 text += " begins to play ";
 text += name;
 text += ".";
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 unsigned int expire = 0;
 if (combat)
  expire = game->getExpiration(1);
 else
  expire = game->getExpiration(BTDice(1, 241, 239).roll());
 d.drawMessage(text.c_str(), game->getDelay());
 for (int i = 0; i < manifest.size(); ++i)
 {
  std::list<BTBaseEffect*> sub = manifest[i]->manifest(d, true, combat, expire, casterLevel, 9, BTTARGET_PARTY, BTTARGET_INDIVIDUAL, true);
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
 s->add("manifest", &manifest, &BTManifest::create);
 s->add("multiManifest", &manifest, &BTMultiManifest::create);
}

void BTSong::readXML(const char *filename, XMLVector<BTSong*> &song)
{
 XMLSerializer parser;
 parser.add("song", &song, &BTSong::create);
 parser.parse(filename, true);
}

