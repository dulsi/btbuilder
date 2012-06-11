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
 : module(m), itemList(".ITM"), jobAbbrevList(&jobList), monsterList(".MON"), spellList(".SPL"), levelMap(NULL), gameTime(0), timedSpecial(-1), delay(1000)
{
 BTDice::Init();
 if (NULL == game)
 {
  game = this;
 }
 BTRace::readXML(m->race, raceList);
 BTSkill::readXML(m->skill, skillList);
 BTXpChart::readXML(m->xpChart, xpChartList);
 BTJob::readXML(m->job, jobList);
 BTPc::readXML("roster.xml", group, roster);
 spellList.load(m->spell);
 itemList.load(m->item);
 monsterList.load(m->monster);
 BTSong::readXML(m->song, songList);
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

XMLVector<BTGroup*> &BTGame::getGroup()
{
 return group;
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

BTSkillList &BTGame::getSkillList()
{
 return skillList;
}

XMLVector<BTSong*> &BTGame::getSongList()
{
 return songList;
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
  if (name == filename)
   return levelMap;
  delete levelMap;
 }
 local.clearAll();
 clearTimedSpecial();
 clearMapEffects();
 int len = strlen(filename);
 if ((len > 4) && (strcmp(".MAP", filename + (len - 4)) == 0))
 {
  BinaryReadFile levelFile(filename);
  levelMap = new BTMap(levelFile);
 }
 else
 {
  levelMap = new BTMap;
  XMLSerializer parser;
  levelMap->serialize(&parser);
  parser.parse(filename, true);
 }
 levelMap->setFilename(filename);
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
 yPos = levelMap->getYSize() - 1 - tmp;
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
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (BTSPELLTYPE_LIGHT == (*itr)->type)
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

const BitField &BTGame::getFlags()
{
 return flags;
}

void BTGame::addFlags(const BitField &flagsToAdd)
{
 flags = flags & flagsToAdd;
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
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (BTSPELLTYPE_DOORDETECT == (*itr)->type)
   bHasDoorDetect = true;
  else if (BTSPELLTYPE_PHASEDOOR == (*itr)->type)
  {
   BTPhaseDoorEffect *phaseDoor = static_cast<BTPhaseDoorEffect*>(*itr);
   if ((phaseDoor->mapX == x) && (phaseDoor->mapY == y) && (phaseDoor->facing == direction))
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

BTChest &BTGame::getChest()
{
 return chest;
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
     if (levelMap->getMonsterChance() >= BTDice(1, 100).roll())
     {
      if (!hasEffectOfType(BTSPELLTYPE_BLOCKENCOUNTERS))
      {
       d.drawText("Random Encounter");
      }
     }
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
      case 'b':
      {
       BTScreenSet cast;
       cast.open("data/play.xml");
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
  clearEffects(d);
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
  while (xPos < 0)
  {
   xPos += levelMap->getXSize();
  }
  xPos = xPos % levelMap->getXSize();
  while (yPos < 0)
  {
   yPos += levelMap->getYSize();
  }
  yPos = yPos % levelMap->getYSize();
  facing = t.facing;
  d.drawView();
  flags.clearAll();
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
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
  {
   if (BTSPELLTYPE_PHASEDOOR == (*itr)->type)
   {
    BTPhaseDoorEffect *phaseDoor = static_cast<BTPhaseDoorEffect*>(*itr);
    if ((phaseDoor->mapX == xPos) && (phaseDoor->mapY == yPos) && (phaseDoor->facing == dir))
    {
     w = 0;
     effect.erase(itr);
     delete phaseDoor;
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
  flags.clearAll();
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

void BTGame::addEffect(BTBaseEffect *e)
{
 if ((BTTIME_COMBAT == e->expiration) || (e->targetsMonsters()))
  combat.addEffect(e);
 else
  effect.push_back(e);
}

void BTGame::clearEffects(BTDisplay &d)
{
 d.stopMusic(BTMUSICID_ALL);
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); itr = effect.begin())
 {
  BTBaseEffect *current = *itr;
  effect.erase(itr);
  if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
   current->finish(d, NULL);
  delete current;
 }
 combat.clearEffects(d);
 d.drawIcons();
}

void BTGame::clearEffectsByType(BTDisplay &d, int type)
{
 bool found = true;
 std::vector<int> musicIds;
 while (found)
 {
  found = false;
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
  {
   if ((*itr)->type == type)
   {
    BTBaseEffect *current = *itr;
    effect.erase(itr);
    bool musicFound = false;
    for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
    {
     if (*itrId == current->musicId)
      musicFound = true;
    }
    if (!musicFound)
     musicIds.push_back(current->musicId);
    if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
     current->finish(d, NULL);
    delete current;
    found = true;
    break;
   }
  }
 }
 checkMusic(d, musicIds);
 d.drawIcons();
}

void BTGame::clearEffectsBySource(BTDisplay &d, bool song)
{
 if (song)
  d.stopMusic(BTMUSICID_ALL);
 bool bFound = true;
 while (bFound)
 {
  bFound = false;
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
  {
   if (((song == true) && ((*itr)->singer != BTTARGET_NOSINGER)) || ((song == false) && ((*itr)->singer == BTTARGET_NOSINGER)))
   {
    BTBaseEffect *current = *itr;
    effect.erase(itr);
    if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
     current->finish(d, NULL);
    delete current;
    bFound = true;
    break;
   }
  }
 }
 d.drawIcons();
}

void BTGame::clearMapEffects()
{
 bool bFound = true;
 while (bFound)
 {
  bFound = false;
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
  {
   if (BTTIME_MAP == (*itr)->expiration)
   {
    BTBaseEffect *current = *itr;
    effect.erase(itr);
//    current->finish(d, NULL);
    delete current;
    bFound = true;
    break;
   }
  }
 }
}

bool BTGame::hasEffectOfType(int type, int group /*= BTTARGET_NONE*/, int target /*= BTTARGET_INDIVIDUAL*/)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->type == type)
  {
   if (group != BTTARGET_NONE)
   {
    if ((*itr)->targets(group, target))
     return true;
    if ((*itr)->targets(group, BTTARGET_INDIVIDUAL))
     return true;
    if ((group != BTTARGET_PARTY) && ((*itr)->targets(BTTARGET_ALLMONSTERS, BTTARGET_INDIVIDUAL)))
     return true;
   }
   else
    return true;
  }
 }
 return combat.hasEffectOfType(type, group, target);
}

void BTGame::addPlayer(BTDisplay &d, int who)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->targets(BTTARGET_PARTY, BTTARGET_INDIVIDUAL))
  {
   (*itr)->apply(d, NULL, BTTARGET_PARTY, who);
  }
 }
 combat.addPlayer(d, who);
}

void BTGame::movedPlayer(BTDisplay &d, int who, int where)
{
 if (where == BTPARTY_REMOVE)
 {
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
  {
   if ((*itr)->targets(BTTARGET_PARTY, who))
   {
    BTBaseEffect *current = *itr;
    itr = effect.erase(itr);
    int size = effect.size();
    if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
     current->finish(d, NULL);
    delete current;
    if (size != effect.size())
     itr = effect.begin();
   }
   else
    ++itr;
  }
 }
 // Must finish combat spells before fixing targets
 combat.movedPlayer(d, who, where);
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (BTPARTY_REMOVE == where)
   (*itr)->remove(&combat, BTTARGET_PARTY, where);
  else
   (*itr)->move(BTTARGET_PARTY, who, where);
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
 std::vector<int> musicIds;
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
 {
  if ((*itr)->isExpired(this))
  {
   BTBaseEffect *current = *itr;
   itr = effect.erase(itr);
   bool musicFound = false;
   for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
   {
    if (*itrId == current->musicId)
     musicFound = true;
   }
   if (!musicFound)
    musicIds.push_back(current->musicId);
   int size = effect.size();
   current->finish(d, combat);
   delete current;
   if (size != effect.size())
    itr = effect.begin();
  }
  else
   ++itr;
 }
 checkMusic(d, musicIds);
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
 {
  if ((*itr)->first)
   (*itr)->first = false;
  else if (BTTIME_PERMANENT != (*itr)->expiration)
   (*itr)->maintain(d, combat);
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
 d.drawIcons();
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

void BTGame::checkMusic(BTDisplay &d, std::vector<int> &musicIds)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->musicId != BTMUSICID_NONE)
  {
   for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
   {
    if (*itrId == (*itr)->musicId)
    {
     musicIds.erase(itrId);
    }
   }
  }
 }
 for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
 {
  d.stopMusic(*itrId);
 }
}

