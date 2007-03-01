/*-------------------------------------------------------------------------*\
  <game.C> -- Game implementation file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "game.h"

BTGame *BTGame::game = NULL;

BTGame::BTGame(const char *itmFile, const char *monFile, const char *splFile)
 : itemList(itmFile), monsterList(monFile), spellList(splFile), levelMap(NULL), xPos(0), yPos(0), facing(BTDIRECTION_NORTH)
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

int BTGame::getFacing()
{
 return facing;
}

int BTGame::getX()
{
 return xPos;
}

int BTGame::getY()
{
 return yPos;
}

int BTGame::getWallType(int x, int y, int direction)
{
 if (x < 0)
  x += 22;
 x = x % 22;
 if (y < 0)
  y += 22;
 y = y % 22;
 IShort w = levelMap->getSquare(y, x).getWall(direction);
 if (w == 2)
  return 2;
 else if (w)
  return 1;
 else
  return 0;
}

void BTGame::moveForward(BTDisplay &d)
{
 xPos += Psuedo3D::changeXY[facing][0] + 22;
 xPos = xPos % 22;
 yPos += Psuedo3D::changeXY[facing][1] + 22;
 yPos = yPos % 22;
 d.drawView();
}

void BTGame::turnLeft(BTDisplay &d)
{
 facing += 3;
 facing = facing % 4;
 d.drawView();
}

void BTGame::turnRight(BTDisplay &d)
{
 facing += 1;
 facing = facing % 4;
 d.drawView();
}

void BTGame::turnAround(BTDisplay &d)
{
 facing += 2;
 facing = facing % 4;
 d.drawView();
}

BTGame *BTGame::getGame()
{
 return game;
}
