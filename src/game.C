/*-------------------------------------------------------------------------*\
  <game.C> -- Game implementation file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "game.h"
#include "ikbbuffer.h"

BTGame *BTGame::game = NULL;

BTGame::BTGame(BTModule *m)
 : module(m), itemList(m->item), monsterList(m->monster), spellList(m->spell), levelMap(NULL)
{
 IRandomize();
 BTJob::readXML(m->job, jobList);
 BTRace::readXML(m->race, raceList);
 PHYSFS_file *start = PHYSFS_openRead(m->start);
 char levelName[14];
 PHYSFS_read(start, levelName, 1, 14);
 loadMap(levelName);
 PHYSFS_uint16 tmp;
 PHYSFS_readULE16(start, &tmp);
 xPos = tmp;
 PHYSFS_readULE16(start, &tmp);
 yPos = 21 - tmp;
 PHYSFS_readULE16(start, &tmp);
 facing = tmp;
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

XMLVector<BTPc*> &BTGame::getRoster()
{
 return roster;
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

XMLVector<BTPc*> &BTGame::getParty()
{
 return party;
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

void BTGame::setFacing(int f)
{
 facing = f;
}

std::string BTGame::getLastInput(void) const
{
 return lastInput;
}

void BTGame::setLastInput(std::string input)
{
 lastInput = input;
}

void BTGame::run(BTDisplay &d)
{
 try
 {
  d.drawFullScreen(module->title, 5000);
  d.setBackground(module->background);
  d.setPsuedo3DConfig(module->wall);
  d.setWallGraphics(0);
  unsigned char key = ' ';
  try
  {
   BTSpecialCommand::Guild.run(d);
  }
  catch (const BTSpecialFlipGoForward &)
  {
   turnAround(d);
   move(d, facing);
  }
  while (true)
  {
   d.drawView();
   d.drawLabel(levelMap->getName());
   key = IKeybufferGet();
   switch (key)
   {
    case 0xBD: // up
     move(d, facing);
     break;
    case 0xBF: // left
     turnLeft(d);
     break;
    case 0xC3: // down
     turnAround(d);
     break;
    case 0xC1: // right
     turnRight(d);
     break;
    case 'q':
     d.clearText();
     d.drawText("Your game will not be saved. Do you want to quit?");
     d.drawText("Yes, or");
     d.drawText("No");
     while (true)
     {
      unsigned char response = IKeybufferGet();
      if (('y' == response) || ('Y' == response))
       throw BTSpecialQuit();
      else if (('n' == response) || ('N' == response))
      {
       d.clearText();
       break;
      }
     }
     break;
    default:
     break;
   }
  }
 }
 catch (const BTSpecialQuit &)
 {
 }
}

void BTGame::move(BTDisplay &d, int dir)
{
 const BTMapSquare& current = levelMap->getSquare(yPos, xPos);
 if (current.getWall(dir) != 1)
 {
  xPos += Psuedo3D::changeXY[dir][0] + 22;
  xPos = xPos % 22;
  yPos += Psuedo3D::changeXY[dir][1] + 22;
  yPos = yPos % 22;
  const BTMapSquare& next = levelMap->getSquare(yPos, xPos);
  IShort s = next.getSpecial();
  d.drawView();
  try
  {
   try
   {
    if (s >= 0)
     levelMap->getSpecial(s)->run(d);
   }
   catch (const BTSpecialTeleport &t) // Hmm... what if another teleport
   {
    loadMap(t.map.c_str()); // Detect if same map
    xPos = t.x;
    yPos = t.y;
    facing = t.facing;
    const BTMapSquare& start = levelMap->getSquare(yPos, xPos);
    d.drawView();
    s = start.getSpecial();
    if ((t.activate) && (s >= 0))
     levelMap->getSpecial(s)->run(d);
   }
  }
  catch (const BTSpecialBack &)
  {
   move(d, (facing + 2) % 4);
  }
  catch (const BTSpecialFlipGoForward &)
  {
   turnAround(d);
   move(d, facing);
  }
  catch (const BTSpecialForward &)
  {
   move(d, facing);
  }
 }
}

void BTGame::turnLeft(BTDisplay &d)
{
 facing += 3;
 facing = facing % 4;
}

void BTGame::turnRight(BTDisplay &d)
{
 facing += 1;
 facing = facing % 4;
}

void BTGame::turnAround(BTDisplay &d)
{
 facing += 2;
 facing = facing % 4;
}

BTGame *BTGame::getGame()
{
 return game;
}
