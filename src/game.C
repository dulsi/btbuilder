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
 BTJob::readXML("data/job.xml", jobList);
 BTRace::readXML("data/race.xml", raceList);
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

XMLVector<BTJob*> &BTGame::getJobList()
{
 return jobList;
}

BTFactory<BTMonster> &BTGame::getMonsterList()
{
 return monsterList;
}

XMLVector<BTRace*> &BTGame::getRaceList()
{
 return raceList;
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
 BinaryReadFile levelFile(filename);
 levelMap = new BTMap(levelFile);
 return levelMap;
}

BTGame *BTGame::getGame()
{
 return game;
}

