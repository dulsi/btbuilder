/*-------------------------------------------------------------------------*\
  <game.C> -- Game implementation file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "game.h"
#include "status.h"

BTGame *BTGame::game = NULL;

BTGame::BTGame(BTModule *m)
 : module(m), itemList(m->item), jobAbbrevList(&jobList), monsterList(m->monster), spellList(m->spell), levelMap(NULL), gameTime(0), timedSpecial(-1), delay(1000)
{
 BTDice::Init();
 if (NULL == game)
 {
  game = this;
 }
 BTRace::readXML(m->race, raceList);
 BTXpChart::readXML(m->xpChart, xpChartList);
 BTJob::readXML(m->job, jobList);
 BTPc::readXML("roster.xml", roster);
 loadStart();
 combat.open("data/combat.xml");
 status.open("data/status.xml");
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

BTJobList &BTGame::getJobList()
{
 return jobList;
}

BTJobAbbrevList &BTGame::getJobAbbrevList()
{
 return jobAbbrevList;
}

BTFactory<BTMonster> &BTGame::getMonsterList()
{
 return monsterList;
}

BTRaceList &BTGame::getRaceList()
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

BTXpChartList &BTGame::getXpChartList()
{
 return xpChartList;
}

BTMap *BTGame::getMap()
{
 return levelMap;
}

BTMap *BTGame::loadMap(const char *filename)
{
 if (levelMap)
 {
  std::string name = levelMap->getFilename();
  name += ".MAP";
  if (name == filename)
   return levelMap;
  delete levelMap;
 }
 local.clearAll();
 clearTimedSpecial();
 clearMapEffects();
 BinaryReadFile levelFile(filename);
 levelMap = new BTMap(levelFile);
 return levelMap;
}

void BTGame::loadStart()
{
 PHYSFS_file *start = PHYSFS_openRead(module->start);
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
}

BTParty &BTGame::getParty()
{
 return party;
}

int BTGame::getLight()
{
 int light = levelMap->getLight();
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); ++itr)
 {
  if (BTSPELLTYPE_LIGHT == spellList[itr->spell].getType())
  {
   if (light < 5)
    light = 5;
  }
 }
 return light;
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
  x += levelMap->getXSize();
 x = x % levelMap->getXSize();
 if (y < 0)
  y += levelMap->getYSize();
 y = y % levelMap->getYSize();
 IShort w = levelMap->getSquare(y, x).getWall(direction);
 bool bHasDoorDetect = false;
 if (w == 0)
  return 0;
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); ++itr)
 {
  if (BTSPELLTYPE_DOORDETECT == spellList[itr->spell].getType())
   bHasDoorDetect = true;
  else if (BTSPELLTYPE_PHASEDOOR == spellList[itr->spell].getType())
  {
   int phaseY = itr->target / (levelMap->getXSize() * BT_DIRECTIONS);
   int phaseX = (itr->target % (levelMap->getXSize() * BT_DIRECTIONS)) / BT_DIRECTIONS;
   int phaseDir = itr->target % BT_DIRECTIONS;
   if ((phaseX == x) && (phaseY == y) && (phaseDir == direction))
    return 0;
  }
 }
 if (w == 2)
  return 2;
 else if (w == 3)
 {
  if (bHasDoorDetect)
  {
   return 2;
  }
  return 1;
 }
 return 1;
}

void BTGame::setFacing(int f)
{
 facing = f;
}

std::string BTGame::getLastInput() const
{
 return lastInput;
}

void BTGame::setLastInput(std::string input)
{
 lastInput = input;
}

int BTGame::getCounter() const
{
 return counter;
}

void BTGame::setCounter(int val)
{
 counter = val;
}

BTCombat &BTGame::getCombat()
{
 return combat;
}

BTStatus &BTGame::getStatus()
{
 return status;
}

bool BTGame::getLocalFlag(int index)
{
 return local.isSet(index);
}

bool BTGame::getGlobalFlag(int index)
{
 return global.isSet(index);
}

void BTGame::setLocalFlag(int index, bool value)
{
 if (value)
  local.set(index);
 else
  local.clear(index);
}

void BTGame::setGlobalFlag(int index, bool value)
{
 if (value)
  global.set(index);
 else
  global.clear(index);
}

void BTGame::run(BTDisplay &d)
{
 bool special = false;
 try
 {
  d.drawFullScreen(module->title, 5000);
  d.refresh();
  d.setPsuedo3DConfig(module->wall);
  d.setWallGraphics(levelMap->getType());
  unsigned char key = ' ';
  try
  {
   BTSpecialCommand::Guild.run(d);
  }
  catch (const BTSpecialFlipGoForward &)
  {
   turnAround(d);
   special = move(d, facing);
  }
  while (true)
  {
   try
   {
    nextTurn(d);
    d.drawView();
    d.drawLabel(levelMap->getName());
    if (special)
    {
     special = false;
     const BTMapSquare& current = levelMap->getSquare(yPos, xPos);
     IShort s = current.getSpecial();
     if (s >= 0)
      special = runSpecial(d, s);
    }
    if ((!special) && (timedSpecial >= 0) && (isExpired(timedExpiration)))
    {
     IShort s = timedSpecial;
     clearTimedSpecial();
     special = runSpecial(d, s);
     continue;
    }
    d.drawView();
    d.drawLabel(levelMap->getName());
    if (!special)
    {
     key = d.readChar(6000);
     switch (key)
     {
      case BTKEY_UP:
       special = move(d, facing);
       break;
      case BTKEY_LEFT:
       turnLeft(d);
       break;
      case BTKEY_DOWN:
       turnAround(d);
       break;
      case BTKEY_RIGHT:
       turnRight(d);
       break;
      case 'q':
       d.clearText();
       d.drawText("Your game will not be saved. Do you want to quit?");
       d.drawText("Yes, or");
       d.drawText("No");
       while (true)
       {
        unsigned char response = d.readChar();
        if (('y' == response) || ('Y' == response))
         throw BTSpecialQuit();
        else if (('n' == response) || ('N' == response))
        {
         d.clearText();
         break;
        }
       }
       break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      {
       int n =  key - '1';
       if (n < party.size())
       {
        status.run(d, party[n]);
       }
       break;
      }
      case 'p':
       combat.clearEncounters();
       combat.run(d, true);
       break;
      case 'c':
      {
       BTScreenSet cast;
       cast.open("data/cast.xml");
       cast.run(d);
       break;
      }
      case 'n':
      {
       BTScreenSet moveTo;
       moveTo.open("data/moveTo.xml");
       moveTo.run(d);
       break;
      }
      default:
       break;
     }
    }
   }
   catch (const BTSpecialDead &)
   {
    loadStart();
    try
    {
     BTSpecialCommand::Guild.run(d);
    }
    catch (const BTSpecialFlipGoForward &)
    {
     turnAround(d);
     special = move(d, facing);
    }
   }
  }
 }
 catch (const BTSpecialQuit &)
 {
 }
}

bool BTGame::runSpecial(BTDisplay &d, IShort special)
{
 try
 {
  levelMap->getSpecial(special)->run(d);
 }
 catch (const BTSpecialTeleport &t)
 {
  loadMap(t.map.c_str());
  d.setWallGraphics(levelMap->getType());
  xPos = t.x;
  yPos = t.y;
  facing = t.facing;
  d.drawView();
  return t.activate;
 }
 catch (const BTSpecialBack &)
 {
  return move(d, (facing + 2) % 4);
 }
 catch (const BTSpecialFlipGoForward &)
 {
  turnAround(d);
  return move(d, facing);
 }
 catch (const BTSpecialForward &)
 {
  return move(d, facing);
 }
 return false;
}

bool BTGame::move(BTDisplay &d, int dir)
{
 const BTMapSquare& current = levelMap->getSquare(yPos, xPos);
 int w = current.getWall(dir);
 if (w != 0)
 {
  for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); ++itr)
  {
   if (BTSPELLTYPE_PHASEDOOR == spellList[itr->spell].getType())
   {
    int phaseY = itr->target / (levelMap->getXSize() * BT_DIRECTIONS);
    int phaseX = (itr->target % (levelMap->getXSize() * BT_DIRECTIONS)) / BT_DIRECTIONS;
    int phaseDir = itr->target % BT_DIRECTIONS;
    if ((phaseX == xPos) && (phaseY == yPos) && (phaseDir == dir))
    {
     w = 0;
     spellEffect.erase(itr);
     break;
    }
   }
  }
 }
 if (w != 1)
 {
  xPos += Psuedo3D::changeXY[dir][0] + levelMap->getXSize();
  xPos = xPos % levelMap->getXSize();
  yPos += Psuedo3D::changeXY[dir][1] + levelMap->getYSize();
  yPos = yPos % levelMap->getYSize();
  return true;
 }
 return false;
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

void BTGame::setTimedSpecial(IShort special, unsigned int expire)
{
 timedExpiration = expire;
 timedSpecial = special;
}

void BTGame::clearTimedSpecial()
{
 timedSpecial = -1;
}

void BTGame::addEffect(int spell, unsigned int expire, int group, int target, BitField &resists)
{
 if ((BTTIME_COMBAT == expire) || (group >= BTTARGET_MONSTER))
  combat.addEffect(spell, expire, group, target, resists);
 else
  spellEffect.push_back(BTSpellEffect(spell, expire, group, target, resists));
}

void BTGame::clearEffects(BTDisplay &d)
{
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); itr = spellEffect.begin())
 {
  int spell = itr->spell;
  int group = itr->group;
  int target = itr->target;
  int expiration = itr->expiration;
  BitField resists = itr->resists;
  spellEffect.erase(itr);
  if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
   spellList[spell].finish(d, NULL, group, target, resists);
 }
 combat.clearEffects(d);
}

void BTGame::clearEffectsByType(BTDisplay &d, int type)
{
 bool bFound = true;
 while (bFound)
 {
  bFound = false;
  for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); ++itr)
  {
   int spell = itr->spell;
   if (spellList[spell].getType() == type)
   {
    int group = itr->group;
    int target = itr->target;
    int expiration = itr->expiration;
    BitField resists = itr->resists;
    spellEffect.erase(itr);
    if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
     spellList[spell].finish(d, NULL, group, target, resists);
    bFound = true;
    break;
   }
  }
 }
}

void BTGame::clearMapEffects()
{
 bool bFound = true;
 while (bFound)
 {
  bFound = false;
  for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end(); ++itr)
  {
   int expiration = itr->expiration;
   if (BTTIME_MAP == expiration)
   {
    int spell = itr->spell;
    int group = itr->group;
    int target = itr->target;
    BitField resists = itr->resists;
    spellEffect.erase(itr);
//    spellList[spell].finish(d, NULL, group, target, resists);
    bFound = true;
    break;
   }
  }
 }
}

void BTGame::addPlayer(BTDisplay &d, int who)
{
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if ((BTTARGET_PARTY == itr->group) && (BTTARGET_INDIVIDUAL == itr->target))
  {
   BitField resists;
   if (spellList[itr->spell].checkResists(NULL, itr->group, who, resists))
   {
    itr->resists.set(who);
    spellList[itr->spell].displayResists(d, NULL, itr->group, who);
   }
   else
    spellList[itr->spell].apply(d, false, NULL, itr->group, who, itr->resists);
  }
  ++itr;
 }
 combat.addPlayer(d, who);
}

void BTGame::movedPlayer(BTDisplay &d, int who, int where)
{
 if (where == BTPARTY_REMOVE)
 {
  for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
  {
   if ((BTTARGET_PARTY == itr->group) && (who == itr->target))
   {
    int spell = itr->spell;
    int expiration = itr->expiration;
    int group = itr->group;
    int target = itr->target;
    BitField resists = itr->resists;
    itr = spellEffect.erase(itr);
    int size = spellEffect.size();
    if ((BTTIME_PERMANENT != expiration) && (BTTIME_CONTINUOUS != expiration))
     spellList[spell].finish(d, NULL, group, target, resists);
    if (size != spellEffect.size())
     itr = spellEffect.begin();
    continue;
   }
   else
    ++itr;
  }
 }
 // Must finish combat spells before fixing targets
 combat.movedPlayer(d, who, where);
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if ((BTTARGET_PARTY == itr->group) && (BTTARGET_INDIVIDUAL == itr->target))
  {
   if (BTPARTY_REMOVE == where)
    itr->resists.remove(where);
   else
    itr->resists.move(who, where);
  }
  else if ((BTTARGET_PARTY == itr->group) && (where != BTPARTY_REMOVE) && (who == itr->target))
  {
   itr->target = where;
  }
  else if ((BTTARGET_PARTY == itr->group) && (where == BTPARTY_REMOVE) && (who < itr->target))
  {
   itr->target--;
  }
  else if ((BTTARGET_PARTY == itr->group) && (who < where) && (where >= itr->target) && (who < itr->target))
  {
   itr->target--;
  }
  else if ((BTTARGET_PARTY == itr->group) && (who > where) && (where <= itr->target) && (who > itr->target))
  {
   itr->target++;
  }
  ++itr;
 }
}

unsigned int BTGame::getExpiration(unsigned int duration)
{
 return (duration + gameTime) % (module->maxTime * BTTIME_MAXDAYS);
}

bool BTGame::isExpired(unsigned int expiration)
{
 if (expiration < ((module->maxTime * BTTIME_MAXDAYS) / 4))
 {
  if ((gameTime < ((module->maxTime * BTTIME_MAXDAYS) / 2)) && (expiration <= gameTime))
   return true;
  else
   return false;
 }
 else if (expiration <= gameTime)
  return true;
 else
  return false;
}

bool BTGame::isDaytime()
{
 return ((gameTime % module->maxTime) < module->nightTime);
}

void BTGame::nextTurn(BTDisplay &d, BTCombat *combat /*= NULL*/)
{
 ++gameTime;
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if (isExpired(itr->expiration))
  {
   int spell = itr->spell;
   int group = itr->group;
   int target = itr->target;
   BitField resists = itr->resists;
   itr = spellEffect.erase(itr);
   int size = spellEffect.size();
   spellList[spell].finish(d, combat, group, target, resists);
   if (size != spellEffect.size())
    itr = spellEffect.begin();
  }
  else
   ++itr;
 }
 for (std::list<BTSpellEffect>::iterator itr = spellEffect.begin(); itr != spellEffect.end();)
 {
  if (itr->first)
   itr->first = false;
  else if (BTTIME_PERMANENT != itr->expiration)
   spellList[itr->spell].maintain(d, combat, itr->group, itr->target, itr->resists);
  ++itr;
 }
 bool spRegen = false;
 if ((0 == gameTime % BTSP_REGEN) && (isDaytime()) && (0 < levelMap->getLight()))
  spRegen = true;
 for (int i = 0; i < party.size(); ++i)
 {
  if (!party[i]->status.isSet(BTSTATUS_DEAD))
  {
   if (party[i]->status.isSet(BTSTATUS_POISONED))
   {
    party[i]->takeHP(1);
   }
   if ((spRegen) && (party[i]->sp < party[i]->maxSp))
    party[i]->sp += 1;
  }
 }
 bool died = party.checkDead(d);
 d.drawStats();
 if (died)
  throw BTSpecialDead();
}

void BTGame::resetTime()
{
 gameTime = 0;
}

int BTGame::getDelay() const
{
 return delay;
}

BTGame *BTGame::getGame()
{
 return game;
}
