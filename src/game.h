#ifndef __GAME_H
#define __GAME_H
/*-------------------------------------------------------------------------*\
  <game.h> -- Game header file

  Date      Programmer  Description
  11/17/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <file.h>
#include "chest.h"
#include "combat.h"
#include "display.h"
#include "factory.h"
#include "item.h"
#include "job.h"
#include "module.h"
#include "monster.h"
#include "race.h"
#include "shop.h"
#include "skill.h"
#include "song.h"
#include "spell.h"
#include "spelleffect.h"
#include "status.h"
#include "map.h"
#include "group.h"
#include "pc.h"
#include "xpchart.h"

class BTPartyDead
{
 public:
  BTPartyDead() {}
};

class BTGame : public Psuedo3DMap, public BTEffectGroup
{
 public:
  BTGame(BTModule *m);
  ~BTGame();

  XMLVector<BTGroup*> &getGroup();
  BTFactory<BTItem> &getItemList();
  BTJobList &getJobList();
  BTJobAbbrevList &getJobAbbrevList();
  BTModule *getModule();
  BTFactory<BTMonster> &getMonsterList();
  BTRaceList &getRaceList();
  XMLVector<BTPc*> &getRoster();
  BTShop *getShop(int id);
  BTSkillList &getSkillList();
  XMLVector<BTSong*> &getSongList();
  BTFactory<BTSpell> &getSpellList();
  BTXpChartList &getXpChartList();
  BTMap *getMap();
  BTMap *loadMap(const char *filename);
  void loadStart();
  BTParty &getParty();

  int getLight();
  int getFacing();
  int getX();
  int getY();
  const BitField &getFlags();
  void addFlags(BTDisplay &d, const BitField &flagsToAdd);
  int getWallType(int x, int y, int direction);
  void setFacing(int f);
  int testWallStrength(int x, int y, int direction);

  std::string getLastInput() const;
  void setLastInput(std::string input);

  int getCounter() const;
  void setCounter(int val);

  BTChest &getChest();
  BTCombat &getCombat();
  BTStatus &getStatus();

  bool getLocalFlag(int index);
  bool getKnowledge(int x, int y);
  bool getGlobalFlag(int index);
  void setLocalFlag(int index, bool value);
  void setKnowledge(int x, int y, bool value);
  void setGlobalFlag(int index, bool value);

  void run(BTDisplay &d);
  bool runSpecial(BTDisplay &d, IShort special);
  bool move(BTDisplay &d, int dir);
  void turnLeft(BTDisplay &d);
  void turnRight(BTDisplay &d);
  void turnAround(BTDisplay &d);

  void setTimedSpecial(IShort special, unsigned int expire);
  void clearTimedSpecial();

  void addEffect(BTBaseEffect *e);
  void checkExpiration(BTDisplay &d, BTCombat *combatObj = NULL);
  void clearEffects(BTDisplay &d);
  void clearEffectsByType(BTDisplay &d, int type);
  void clearEffectsBySource(BTDisplay &d, bool song, int group = BTTARGET_NONE, int target = BTTARGET_INDIVIDUAL);
  bool hasEffectOfType(int type, int group = BTTARGET_NONE, int target = BTTARGET_INDIVIDUAL);
  void addPlayer(BTDisplay &d, int who);
  void movedPlayer(BTDisplay &d, int who, int where);
  void movedPlayer(BTDisplay &d, BTCombat *combatObj, int who, int where);
  unsigned int getExpiration(unsigned int duration);
  bool isExpired(unsigned int expiration);
  bool isDaytime();
  void nextTurn(BTDisplay &d, BTCombat *combat = NULL);
  void resetTime();

  int getDelay() const;

  void save();

  static BTGame *getGame();

 private:
  BTModule *module;
  BTFactory<BTItem> itemList;
  BTJobList jobList;
  BTJobAbbrevList jobAbbrevList;
  BTFactory<BTMonster> monsterList;
  BTRaceList raceList;
  XMLVector<BTGroup*> group;
  XMLVector<BTPc*> roster;
  XMLVector<BTShop*> shops;
  BTSkillList skillList;
  XMLVector<BTSong*> songList;
  BTFactory<BTSpell> spellList;
  BTXpChartList xpChartList;
  BTMap *levelMap;
  int xPos, yPos, facing;
  BitField flags;
  BTParty party;
  std::string lastInput;
  int counter;
  BTChest chest;
  BTCombat combat;
  BTStatus status;
  BitField local;
  BitField global;
  BitField knowledge;
  unsigned int gameTime;
  unsigned int timedExpiration;
  IShort timedSpecial;
  int delay;
  Psuedo3DConfig *p3dConfig;

  static BTGame *game;
};

#endif

