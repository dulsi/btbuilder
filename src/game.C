/*-------------------------------------------------------------------------*\
  <game.C> -- Game implementation file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "game.h"
#include "status.h"

BTCore *BTCore::core = NULL;
BTGame *BTGame::game = NULL;

BTCore::BTCore(BTModule *m)
 : module(m), itemList(".ITM"), monsterList(".MON"), spellList(".SPL"), levelMap(NULL)
{
 if (NULL == core)
 {
  core = this;
 }
 BTRace::readXML(m->race, raceList);
 BTSkill::readXML(m->skill, skillList);
 BTXpChart::readXML(m->xpChart, xpChartList);
 BTJob::readXML(m->job, jobList);
 BTShop::readXML("shops.xml", shops);
 spellList.load(m->spell);
 itemList.load(m->item);
 monsterList.load(m->monster);
 BTSong::readXML(m->song, songList);
 Psuedo3DConfig::readXML(m->wall, p3dConfigList);
}

BTCore::~BTCore()
{
 if (levelMap)
 {
  delete levelMap;
 }
 if (core == this)
 {
  core = NULL;
 }
}

BTFactory<BTItem> &BTCore::getItemList()
{
 return itemList;
}

BTJobList &BTCore::getJobList()
{
 return jobList;
}

BTModule *BTCore::getModule()
{
 return module;
}

BTFactory<BTMonster> &BTCore::getMonsterList()
{
 return monsterList;
}

Psuedo3DConfigList &BTCore::getPsuedo3DConfigList()
{
 return p3dConfigList;
}

BTRaceList &BTCore::getRaceList()
{
 return raceList;
}

BTShop *BTCore::getShop(int id)
{
 for (int i = 0; i < shops.size(); ++i)
 {
  if (shops[i]->id == id)
  {
   return shops[i];
  }
 }
 BTShop *shop = new BTShop;
 shop->id = id;
 shop->initDefault();
 shops.push_back(shop);
 return shop;
}

BTSkillList &BTCore::getSkillList()
{
 return skillList;
}

XMLVector<BTSong*> &BTCore::getSongList()
{
 return songList;
}

BTFactory<BTSpell, BTSpell1> &BTCore::getSpellList()
{
 return spellList;
}

BTXpChartList &BTCore::getXpChartList()
{
 return xpChartList;
}

BTMap *BTCore::getMap()
{
 return levelMap;
}

BTMap *BTCore::loadMap(const char *filename)
{
 std::string finalname = filename;
 int len = strlen(filename);
 if ((len > 4) && (strcmp(".MAP", filename + (len - 4)) == 0))
 {
  char tmp[len + 1];
  strcpy(tmp, filename);
  strcpy(tmp + len - 3, "xml");
  if (0 != PHYSFS_exists(tmp))
  {
   finalname = tmp;
   len = finalname.length();
  }
 }
 if (levelMap)
 {
  std::string name = levelMap->getFilename();
  if (name == finalname)
   return levelMap;
  delete levelMap;
 }
 if ((len > 4) && (strcmp(".MAP", finalname.c_str() + (len - 4)) == 0))
 {
  BinaryReadFile levelFile(finalname.c_str());
  levelMap = new BTMap(levelFile);
 }
 else
 {
  levelMap = new BTMap(1); // Assume version 1 file unless version is in the file.
  XMLSerializer parser;
  levelMap->serialize(&parser);
  parser.parse(finalname.c_str(), true);
  levelMap->upgrade();
 }
 levelMap->setFilename(finalname.c_str());
 return levelMap;
}

int BTCore::getMapType(int x, int y, int direction)
{
 return getMap()->getSquare(y, x).getWall(direction);
}

int BTCore::getXSize() const
{
 return levelMap->getXSize();
}

int BTCore::getYSize() const
{
 return levelMap->getYSize();
}

bool BTCore::hasSpecial(int x, int y)
{
 return (levelMap->getSquare(y, x).getSpecial() >= 0);
}

BTCore *BTCore::getCore()
{
 return core;
}

BTGame::BTGame(BTModule *m)
 : BTCore(m), jobAbbrevList(&jobList), gameTime(0), timedSpecial(-1), delay(1000)
{
 BTDice::Init();
 if (NULL == game)
 {
  game = this;
 }
 BTPc::readXML("roster.xml", group, roster);
 loadStart();
 combat.open("data/combat.xml");
 status.open("data/status.xml");
}

BTGame::~BTGame()
{
 if (game == this)
 {
  game = NULL;
 }
}

XMLVector<BTGroup*> &BTGame::getGroup()
{
 return group;
}

BTJobAbbrevList &BTGame::getJobAbbrevList()
{
 return jobAbbrevList;
}

XMLVector<BTPc*> &BTGame::getRoster()
{
 return roster;
}

BTMap *BTGame::loadMap(const char *filename, bool clearState /*= true*/)
{
 if (levelMap)
 {
  std::string name = levelMap->getFilename();
  if (name == filename)
   return levelMap;
  int len = strlen(filename);
  if ((len > 4) && (strcmp(".MAP", filename + (len - 4)) == 0))
  {
   char tmp[len + 1];
   strcpy(tmp, filename);
   strcpy(tmp + len - 3, "xml");
   if (name == tmp)
   {
    return levelMap;
   }
  }
 }
 if (clearState == false)
 {
  local.clearAll();
  knowledge.clearAll();
  clearTimedSpecial();
  clearMapEffects();
 }
 BTCore::loadMap(filename);
}

void BTGame::loadStart()
{
 if (module->startMap.empty())
 {
  PHYSFS_file *start = PHYSFS_openRead("START.BRD");
  char levelName[15];
  levelName[14] = 0; // Ensure it ends with a null
  PHYSFS_read(start, levelName, 1, 14);
  module->startMap = levelName;
  PHYSFS_uint16 tmp;
  PHYSFS_readULE16(start, &tmp);
  module->startX = tmp;
  PHYSFS_readULE16(start, &tmp);
  module->startY = tmp;
  PHYSFS_readULE16(start, &tmp);
  module->startFace = tmp;
 }
 loadMap(module->startMap.c_str());
 xPos = module->startX;
 yPos = levelMap->getYSize() - 1 - module->startY;
 facing = module->startFace;
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
   BTLightEffect *l = dynamic_cast<BTLightEffect*>(*itr);
   if (l)
   {
    if (l->illumination > light)
     light = l->illumination;
   }
   else if (light < 5)
    light = 5;
  }
 }
 return light;
}

const BitField &BTGame::getFlags()
{
 return flags;
}

void BTGame::addFlags(BTDisplay &d, const BitField &flagsToAdd)
{
 flags |= flagsToAdd;
 bool effectChange = false;
 if (flagsToAdd.isSet(BTSPECIALFLAG_DARKNESS))
 {
  clearEffectsByType(d, BTSPELLTYPE_LIGHT);
  effectChange = true;
 }
 if (flagsToAdd.isSet(BTSPECIALFLAG_SILENCE))
 {
  clearEffectsBySource(d, true);
  effectChange = true;
 }
 if (flagsToAdd.isSet(BTSPECIALFLAG_ANTIMAGIC))
 {
  clearEffectsBySource(d, false);
  effectChange = true;
 }
 if (effectChange)
 {
  checkExpiration(d, &combat);
  d.drawIcons();
 }
}

int BTGame::getWallType(int x, int y, int direction)
{
 rationalize(x, y);
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
 int mapType = p3dConfig->findMapType(w, true);
 if (mapType == 0)
  return 0;
 if (bHasDoorDetect)
 {
  if (-1 != p3dConfig->mapType[mapType - 1]->viewType)
   return p3dConfig->mapType[mapType - 1]->viewType;
 }
 else
 {
  if (-1 != p3dConfig->mapType[mapType - 1]->incompleteType)
   return p3dConfig->mapType[mapType - 1]->incompleteType;
 }
 return w;
}

void BTGame::setFacing(int f)
{
 facing = f;
}

int BTGame::testWallStrength(int x, int y, int direction)
{
 if (x < 0)
  x += levelMap->getXSize();
 x = x % levelMap->getXSize();
 if (y < 0)
  y += levelMap->getYSize();
 y = y % levelMap->getYSize();
 IShort w = levelMap->getSquare(y, x).getWall(direction);
 if (w == 0)
  return 0;
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (BTSPELLTYPE_PHASEDOOR == (*itr)->type)
  {
   BTPhaseDoorEffect *phaseDoor = static_cast<BTPhaseDoorEffect*>(*itr);
   if ((phaseDoor->mapX == x) && (phaseDoor->mapY == y) && (phaseDoor->facing == direction))
    return 0;
  }
 }
 int mapType = p3dConfig->findMapType(w, true);
 if (mapType == 0)
  return 0;
 if (p3dConfig->mapType[mapType - 1]->invincible)
  return 2;
 else
  return 1;
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

int BTGame::getKnowledge(int x, int y)
{
 int index = y * levelMap->getXSize() + x;
 if (knowledge.isSet(index))
  return BTKNOWLEDGE_YES;
 else
  return BTKNOWLEDGE_NO;
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

void BTGame::setKnowledge(int x, int y, bool value)
{
 int index = y * levelMap->getXSize() + x;
 if (value)
  knowledge.set(index);
 else
  knowledge.clear(index);
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
  d.setPsuedo3DConfig(&p3dConfigList);
  bool skipGuild = false;
  if (PHYSFS_exists("savegame.xml"))
  {
   d.clearText();
   d.addText("Do you wish to restore your last saved game?");
   d.addChoice("yY", "Yes");
   d.addChoice("nN", "No");
   unsigned int key = d.process();
   if (('y' == key) || ('Y' == key))
   {
    readSaveXML("savegame.xml");
    skipGuild = true;
   }
   d.clearText();
  }
  p3dConfig = d.setWallGraphics(levelMap->getType());
  unsigned char key = ' ';
  if (!skipGuild)
  {
   setKnowledge(xPos, yPos, true);
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
  while (true)
  {
   try
   {
    setKnowledge(xPos, yPos, true);
    nextTurn(d);
    d.drawView();
    d.drawLabel(levelMap->getName());
    if ((flags.isSet(BTSPECIALFLAG_DARKNESS)) && (hasEffectOfType(BTSPELLTYPE_LIGHT)))
    {
     SDL_Delay(500); // pause for bit to show a flash of light.
     clearEffectsByType(d, BTSPELLTYPE_LIGHT);
     checkExpiration(d, &combat);
     d.drawView();
     d.drawIcons();
    }
    if (special)
    {
     special = false;
     const BTMapSquare& current = levelMap->getSquare(yPos, xPos);
     IShort s = current.getSpecial();
     if (s >= 0)
      special = runSpecial(d, s);
    }
    setKnowledge(xPos, yPos, true);
    if ((!special) && (timedSpecial >= 0) && (isExpired(timedExpiration)))
    {
     IShort s = timedSpecial;
     clearTimedSpecial();
     special = runSpecial(d, s);
     continue;
    }
    setKnowledge(xPos, yPos, true);
    d.drawView();
    d.drawLabel(levelMap->getName());
    if (!special)
    {
     if (!hasEffectOfType(BTSPELLTYPE_BLOCKENCOUNTERS))
     {
      levelMap->checkRandomEncounter(d);
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
      case 'd':
      {
       BTScreenSet drop;
       drop.open("data/dismiss.xml");
       drop.run(d);
       break;
      }
      case 'u':
      {
       BTScreenSet useItem;
       useItem.open("data/use.xml");
       useItem.run(d);
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
      case 's':
      {
       BTScreenSet saveGame;
       saveGame.open("data/savegame.xml");
       saveGame.run(d);
       break;
      }
      case '?':
      {
       int quarter = (gameTime / (module->maxTime / 96)) % 4;
       int hour = ((gameTime / (module->maxTime / 24)) + 6) % 24 + ((quarter == 3) ? 1 : 0);
       std::string timeText;
       if (hour == 0)
       {
        timeText = "midnight.";
       }
       else if (hour == 12)
       {
        timeText = "noon.";
       }
       else if (hour < 12)
       {
        char tmp[20];
        sprintf(tmp, "%d a.m.", hour);
        timeText = tmp;
       }
       else
       {
        char tmp[20];
        sprintf(tmp, "%d p.m.", hour - 12);
        timeText = tmp;
       }
       switch (quarter)
       {
        case 1:
         timeText = "quarter after " + timeText;
         break;
        case 2:
         timeText = "half past " + timeText;
         break;
        case 3:
         timeText = "quarter til " + timeText;
         break;
        case 0:
        default:
         break;
       }
       timeText = "The time is " + timeText;
       d.drawText(timeText.c_str());
       d.drawView();
       d.drawMap(false);
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
  checkExpiration(d, &combat);
  d.drawIcons();
 }
 d.setPsuedo3DConfig(NULL);
}

bool BTGame::runSpecial(BTDisplay &d, IShort special)
{
 try
 {
  BTSpecial* sp = levelMap->getSpecial(special);
  if (sp)
   sp->run(d);
 }
 catch (const BTSpecialTeleport &t)
 {
  loadMap(t.map.c_str());
  p3dConfig = d.setWallGraphics(levelMap->getType());
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
     break;
    }
   }
  }
 }
 bool walk = true;
 if (w != 0)
 {
  int mapType = p3dConfig->findMapType(w, true);
  if (0 != mapType)
  {
   if (!p3dConfig->mapType[mapType - 1]->passable)
    walk = false;
  }
 }
 if (walk)
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
 {
  combat.addEffect(e);
 }
 else
 {
  BTEffectGroup::addEffect(e);
 }
}

void BTGame::checkExpiration(BTDisplay &d, BTCombat *combatObj /*= NULL*/)
{
 combat.checkExpiration(d, &combat);
 BTEffectGroup::checkExpiration(d, &combat);
}

void BTGame::clearEffects(BTDisplay &d)
{
 combat.clearEffects(d);
 BTEffectGroup::clearEffects(d);
}

void BTGame::clearEffectsByType(BTDisplay &d, int type)
{
 combat.clearEffectsByType(d, type);
 BTEffectGroup::clearEffectsByType(d, type);
}

void BTGame::clearEffectsBySource(BTDisplay &d, bool song, int group /*= BTTARGET_NONE*/, int target /*= BTTARGET_INDIVIDUAL*/)
{
 combat.clearEffectsBySource(d, song, group, target);
 BTEffectGroup::clearEffectsBySource(d, song, group, target);
}

bool BTGame::hasEffectOfType(int type, int group /*= BTTARGET_NONE*/, int target /*= BTTARGET_INDIVIDUAL*/)
{
 if (BTEffectGroup::hasEffectOfType(type, group, target))
  return true;
 return combat.hasEffectOfType(type, group, target);
}

void BTGame::addPlayer(BTDisplay &d, int who)
{
 BTEffectGroup::addPlayer(d, who);
 combat.addPlayer(d, who);
}

void BTGame::movedPlayer(BTDisplay &d, int who, int where)
{
 movedPlayer(d, &combat, who, where);
}

void BTGame::movedPlayer(BTDisplay &d, BTCombat *combatObj, int who, int where)
{
 combatObj->movedPlayer(d, combatObj, who, where);
 BTEffectGroup::movedPlayer(d, combatObj, who, where);
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
 checkExpiration(d, combat);
 maintain(d, combat);
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

int *BTGame::getDelay()
{
 return &delay;
}

void BTGame::save()
{
 BTPc::writeXML("roster.xml", getGroup(), getRoster());
 BTShop::writeXML("shops.xml", shops);
}

void BTGame::serialize(ObjectSerializer *s, BTGroup &curParty, std::string &startMap)
{
 s->add("party", &getGroup(), &BTGroup::create);
 s->add("pc", &getRoster(), &BTPc::create);
 s->add("startMap", &startMap);
 s->add("xPos", &xPos);
 s->add("yPos", &yPos);
 s->add("facing", &facing);
 s->add("curParty", &curParty);
 s->add("counter", &counter);
 s->add("gameTime", &gameTime);
 s->add("global", &global, NULL);
 s->add("local", &local, NULL);
 s->add("knowledge", &knowledge, NULL);
 s->add("timedExpiration", &timedExpiration);
 s->add("timedSpecial", &timedSpecial);
 s->add("baseeffect", typeid(BTBaseEffect).name(), &effect, &BTBaseEffect::create);
 s->add("targetedeffect", typeid(BTTargetedEffect).name(), &effect, &BTTargetedEffect::create);
 s->add("resistedeffect", typeid(BTResistedEffect).name(), &effect, &BTResistedEffect::create);
 s->add("attackeffect", typeid(BTAttackEffect).name(), &effect, &BTAttackEffect::create);
 s->add("curestatuseffect", typeid(BTCureStatusEffect).name(), &effect, &BTCureStatusEffect::create);
 s->add("healeffect", typeid(BTHealEffect).name(), &effect, &BTHealEffect::create);
 s->add("summonmonstereffect", typeid(BTSummonMonsterEffect).name(), &effect, &BTSummonMonsterEffect::create);
 s->add("summonillusioneffect", typeid(BTSummonIllusionEffect).name(), &effect, &BTSummonIllusionEffect::create);
 s->add("dispellillusioneffect", typeid(BTDispellIllusionEffect).name(), &effect, &BTDispellIllusionEffect::create);
 s->add("armorbonuseffect", typeid(BTArmorBonusEffect).name(), &effect, &BTArmorBonusEffect::create);
 s->add("hitbonuseffect", typeid(BTHitBonusEffect).name(), &effect, &BTHitBonusEffect::create);
 s->add("resurrecteffect", typeid(BTResurrectEffect).name(), &effect, &BTResurrectEffect::create);
 s->add("dispellmagiceffect", typeid(BTDispellMagicEffect).name(), &effect, &BTDispellMagicEffect::create);
 s->add("phasedooreffect", typeid(BTPhaseDoorEffect).name(), &effect, &BTPhaseDoorEffect::create);
 s->add("regenskilleffect", typeid(BTRegenSkillEffect).name(), &effect, &BTRegenSkillEffect::create);
 s->add("pusheffect", typeid(BTPushEffect).name(), &effect, &BTPushEffect::create);
 s->add("attackratebonuseffect", typeid(BTAttackRateBonusEffect).name(), &effect, &BTAttackRateBonusEffect::create);
 s->add("regenmanaeffect", typeid(BTRegenManaEffect).name(), &effect, &BTRegenManaEffect::create);
 s->add("savebonuseffect", typeid(BTSaveBonusEffect).name(), &effect, &BTSaveBonusEffect::create);
 s->add("scrysighteffect", typeid(BTScrySightEffect).name(), &effect, &BTScrySightEffect::create);
 s->add("spellbindeffect", typeid(BTSpellBindEffect).name(), &effect, &BTSpellBindEffect::create);
 s->add("lighteffect", typeid(BTLightEffect).name(), &effect, &BTLightEffect::create);
}

void BTGame::readSaveXML(const char *filename)
{
 std::string startMap;
 BTGroup curParty;
 XMLSerializer parser;
 party.erase(party.begin(), party.end());
 group.erase(group.begin(), group.end());
 roster.erase(roster.begin(), roster.end());
 serialize(&parser, curParty, startMap);
 parser.parse(filename, true);
 for (int i = 0; i < getRoster().size(); ++i)
  getRoster()[i]->updateSkills();
 loadMap(startMap.c_str(), false);
 for (int i = 0; i < curParty.member.size(); ++i)
 {
  for (int k = 0; k < roster.size(); ++k)
  {
   if (0 == strcmp(roster[k]->name, curParty.member[i].c_str()))
   {
    party.push_back(roster[k]);
   }
  }
 }
}

void BTGame::writeSaveXML(const char *filename)
{
 std::string startMap = levelMap->getFilename();
 BTGroup curParty;
 curParty.name = "Current Party";
 for (int i = 0; i < party.size(); ++i)
  curParty.member.push_back(party[i]->name);
 XMLSerializer parser;
 serialize(&parser, curParty, startMap);
 parser.write(filename, true);
}

BTGame *BTGame::getGame()
{
 return game;
}

