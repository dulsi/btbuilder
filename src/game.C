/*-------------------------------------------------------------------------*\
  <game.C> -- Game implementation file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "game.h"

BTGame *BTGame::game = NULL;

BTGame::BTGame(const char *itmFile, const char *monFile, const char *splFile)
 : itemList(itmFile), monsterList(monFile), spellList(splFile), levelMap(NULL)
{
 if (NULL == game)
 {
  game = this;
 }
}

BTGame::~BTGame()
{
 if (levelMap)
 {
  delete levelMap;
 }
 if (game == this)
 {
  game = NULL;
 }
}

BTFactory<BTItem> &BTGame::getItemList()
{
 return itemList;
}

BTFactory<BTMonster> &BTGame::getMonsterList()
{
 return monsterList;
}

BTFactory<BTSpell> &BTGame::getSpellList()
{
 return spellList;
}

BTMap *BTGame::getMap()
{
 return levelMap;
}

BTMap *BTGame::loadMap(const char *filename)
{
 if (levelMap)
 {
  delete levelMap;
 }
 levelMap = new BTMap(filename);
 return levelMap;
}

BTGame *BTGame::getGame()
{
 return game;
}

