/*-------------------------------------------------------------------------*\
  <monster.C> -- Monster implementation file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "monster.h"
#include "game.h"

#define MONSTER_RANGEDTYPENONE   0
#define MONSTER_RANGEDTYPEONEFOE 1
#define MONSTER_RANGEDTYPEGROUP  2
#define MONSTER_RANGEDTYPEMAGIC  3

BTMonster::BTMonster(BinaryReadFile &f)
 : gender(BTGENDER_MALE), wandering(true)
{
 IUByte unknown;
 char tmp[15];
 IShort num;
 IShort actions[4];

 f.readUByteArray(14, (IUByte *)tmp);
 tmp[15] = 0;
 name = tmp;
 pluralName = name + "(s)";
 f.readShort(startDistance);
 startDistance = startDistance / 16;
 f.readShort(move);
 f.readShort(rateAttacks);
 f.readShort(illusion);
 f.readShort(num);
 picture.value = num;
 f.readShortArray(4, actions);
 combatAction.push_back(actions[0]);
 if ((actions[0] != actions[1]) || (actions[0] != actions[2]) || (actions[0] != actions[3]))
 {
  combatAction.push_back(actions[1]);
  if ((actions[0] != actions[2]) || (actions[1] != actions[3]))
  {
   combatAction.push_back(actions[2]);
   combatAction.push_back(actions[3]);
  }
 }
 f.readShort(num);
 meleeExtra = num;
 IShort calcAc;
 f.readShort(calcAc);
 ac = (calcAc * -1) + 10;
 f.readShort(maxAppearing);
 hp.read(f);
 meleeDamage.read(f);
 f.readUByteArray(14, (IUByte *)tmp);
 tmp[15] = 0;
 meleeMessage = new char[strlen(tmp) + 1];
 strcpy(meleeMessage, tmp);
 f.readShort(magicResistance);
 rangedDamage.read(f);
 f.readUByte(unknown);
 f.readShort(num);
 rangedExtra = num;
 f.readShort(range);
 f.readUByteArray(14, (IUByte *)tmp);
 tmp[15] = 0;
 rangedMessage = new char[strlen(tmp) + 1];
 strcpy(rangedMessage, tmp);
 f.readShort(num);
 rangedType = num;
 if (rangedType >= MONSTER_RANGEDTYPEMAGIC)
 {
  rangedSpell = rangedType - MONSTER_RANGEDTYPEMAGIC;
  rangedType = MONSTER_RANGEDTYPEMAGIC;
 }
 f.readShort(level);
 gold.read(f);
 f.readUByte(unknown);
 xp = calcXp();
}

BTMonster::BTMonster()
 : gender(BTGENDER_MALE), level(0), startDistance(0), move(1), rateAttacks(1), illusion(0), ac(0), maxAppearing(1), magicResistance(0), wandering(true), meleeExtra(BTEXTRADAMAGE_NONE), rangedType(0), rangedSpell(0), rangedExtra(BTEXTRADAMAGE_NONE), range(0), xp(0)
{
 meleeMessage = new char[1];
 meleeMessage[0] = 0;
 rangedMessage = new char[1];
 rangedMessage[0] = 0;
 xp = calcXp();
}

BTMonster::BTMonster(const BTMonster &copy)
 : name(copy.name), pluralName(copy.pluralName), gender(copy.gender), level(copy.level), startDistance(copy.startDistance),
 move(copy.move), rateAttacks(copy.rateAttacks), illusion(copy.illusion), picture(copy.picture),
 combatAction(copy.combatAction), ac(copy.ac), maxAppearing(copy.maxAppearing), hp(copy.hp), gold(copy.gold),
 magicResistance(copy.magicResistance), wandering(copy.wandering), xp(copy.xp), meleeDamage(copy.meleeDamage),
 meleeExtra(copy.meleeExtra), rangedType(copy.rangedType), rangedSpell(copy.rangedSpell), rangedDamage(copy.rangedDamage),
 rangedExtra(copy.rangedExtra), range(copy.range)
{
 meleeMessage = new char[strlen(copy.meleeMessage) + 1];
 strcpy(meleeMessage, copy.meleeMessage);
 rangedMessage = new char[strlen(copy.rangedMessage) + 1];
 strcpy(rangedMessage, copy.rangedMessage);
}

BTMonster::~BTMonster()
{
 if (meleeMessage)
  delete [] meleeMessage;
 if (rangedMessage)
  delete [] rangedMessage;
}

unsigned int BTMonster::calcXp() const
{
 return rateAttacks * meleeDamage.getMax() + ac + hp.getMax();
}

const std::string &BTMonster::getName() const
{
 return name;
}

const std::string &BTMonster::getPluralName() const
{
 return pluralName;
}

IShort BTMonster::getAc() const
{
 return ac;
}

IShort BTMonster::getCombatAction(IShort round) const
{
 return combatAction[round % combatAction.size()];
}

int BTMonster::getGender() const
{
 return gender;
}

const BTDice &BTMonster::getGold() const
{
 return gold;
}

const BTDice &BTMonster::getHp() const
{
 return hp;
}

IShort BTMonster::getLevel() const
{
 return level;
}

IShort BTMonster::getMagicResistance() const
{
 return magicResistance;
}

IShort BTMonster::getMaxAppearing() const
{
 return maxAppearing;
}

const BTDice &BTMonster::getMeleeDamage() const
{
 return meleeDamage;
}

IShort BTMonster::getMeleeExtra() const
{
 return meleeExtra;
}

const char *BTMonster::getMeleeMessage() const
{
 return meleeMessage;
}

IShort BTMonster::getMove() const
{
 return move;
}

IShort BTMonster::getPicture() const
{
 return picture.value;
}

IShort BTMonster::getRange() const
{
 return range;
}

const BTDice &BTMonster::getRangedDamage() const
{
 return rangedDamage;
}

IShort BTMonster::getRangedExtra() const
{
 return rangedExtra;
}

const char *BTMonster::getRangedMessage() const
{
 return rangedMessage;
}

IShort BTMonster::getRangedSpell() const
{
 return rangedSpell;
}

IShort BTMonster::getRangedType() const
{
 return rangedType;
}

IShort BTMonster::getRateAttacks() const
{
 return rateAttacks;
}

IShort BTMonster::getStartDistance() const
{
 return startDistance;
}

unsigned int BTMonster::getXp() const
{
 return xp;
}

IBool BTMonster::isIllusion() const
{
 return illusion;
}

bool BTMonster::isWandering() const
{
 return wandering;
}

bool BTMonster::savingThrow(int difficulty /*= BTSAVE_DIFFICULTY*/) const
{
 BTJobList &jobList = BTGame::getGame()->getJobList();
 int job = (isIllusion() ? BTJOB_ILLUSION : BTJOB_MONSTER);
 int save = jobList[job]->calcSave(level);
 int roll = BTDice(1, 20, save).roll();
 if (roll == 20 + save)
  return true;
 else if (roll == 1 + save)
  return false;
 else
  return (roll >= difficulty);
}

void BTMonster::setName(const std::string &nm)
{
 name = nm;
}

void BTMonster::setPluralName(const std::string &nm)
{
 pluralName = nm;
}

void BTMonster::setLevel(IShort l)
{
 level = l;
}

void BTMonster::setPicture(IShort pic)
{
 picture.value = pic;
}

void BTMonster::setStartDistance(IShort d)
{
 startDistance = d;
}

void BTMonster::useRangedOnGroup(BTDisplay &d, BTCombatantCollection *grp, int distance, int &activeNum)
{
 BTGame *game = BTGame::getGame();
 std::string text = getName();
 text += " ";
 text += getRangedMessage();
 d.drawMessage(text.c_str(), game->getDelay());
 if (distance > getRange())
  return;
 bool farRange = false;
 for (int i = 0; i < grp->size(); ++i)
 {
  if (grp->at(i)->isAlive())
  {
   text = BTCombatant::specialAttack(grp->at(i), getRangedDamage(), getRangedExtra(), farRange, activeNum);
   d.drawMessage(text.c_str(), game->getDelay());
  }
 }
}

void BTMonster::write(BinaryWriteFile &f)
{
 IUByte unknown;
 IShort value;
 char tmp[14];
 IShort actions[4];

 strncpy(tmp, name.c_str(), 14);
 f.writeUByteArray(14, (IUByte *)tmp);
 value = startDistance * 16;
 f.writeShort(value);
 f.writeShort(move);
 f.writeShort(rateAttacks);
 f.writeShort(illusion);
 value = picture.value;
 f.writeShort(value);
 for (int i = 0; i < 4; ++i)
  actions[i] = getCombatAction(i);
 f.writeShortArray(4, actions);
 value = meleeExtra;
 f.writeShort(value);
 IShort calcAc;
 calcAc = (ac - 10) * -1;
 f.writeShort(calcAc);
 f.writeShort(maxAppearing);
 hp.write(f);
 meleeDamage.write(f);
 strncpy(tmp, meleeMessage, 14);
 f.writeUByteArray(14, (IUByte *)tmp);
 f.writeShort(magicResistance);
 rangedDamage.write(f);
 f.writeUByte(unknown);
 value = rangedExtra;
 f.writeShort(value);
 f.writeShort(range);
 strncpy(tmp, rangedMessage, 14);
 f.writeUByteArray(14, (IUByte *)tmp);
 value = rangedType;
 if (rangedType == MONSTER_RANGEDTYPEMAGIC)
 {
  value += rangedSpell;
 }
 f.writeShort(value);
 f.writeShort(level);
 gold.write(f);
 f.writeUByte(unknown);
}

void BTMonster::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("pluralName", &pluralName);
 s->add("gender", &gender, NULL, &genderLookup);
 s->add("level", &level);
 s->add("startDistance", &startDistance);
 s->add("move", &move);
 s->add("rateAttacks", &rateAttacks);
 s->add("illusion", &illusion);
 s->add("picture", &picture);
 s->add("combatAction", &combatAction, &combatActionLookup);
 s->add("ac", &ac);
 s->add("maxAppearing", &maxAppearing);
 s->add("hp", &hp);
 s->add("gold", &gold);
 s->add("magicResistance", &magicResistance);
 s->add("wandering", &wandering);
 s->add("xp", &xp);
 s->add("meleeMessage", &meleeMessage);
 s->add("meleeDamage", &meleeDamage);
 s->add("meleeExtra", &meleeExtra, NULL, &extraDamageLookup);
 s->add("rangedMessage", &rangedMessage);
 s->add("rangedType", &rangedType, NULL, &rangedTypeLookup);
 s->add("rangedSpell", &rangedSpell);
 s->add("rangedSpellName", &rangedSpell, NULL, &BTCore::getCore()->getSpellList(), -1, "(none)");
 s->add("rangedDamage", &rangedDamage);
 s->add("rangedExtra", &rangedExtra, NULL, &extraDamageLookup);
 s->add("range", &range);
}

void BTMonster::readXML(const char *filename, XMLVector<BTMonster*> &monster)
{
 XMLSerializer parser;
 parser.add("monster", &monster, &BTMonster::create);
 parser.parse(filename, true);
 if (monster[0]->getXp() == 4)
 {
  for (int i = 0; i < monster.size(); ++i)
   monster[i]->xp = monster[i]->calcXp();
 }
}

void BTMonster::writeXML(const char *filename, XMLVector<BTMonster*> &monster)
{
 XMLSerializer parser;
 parser.add("monster", &monster, &BTMonster::create);
 parser.write(filename, true);
}

int BTMonsterListCompare::Compare(const BTMonster &a, const BTMonster &b) const
{
 int ans = a.getLevel() - b.getLevel();
 if (0 == ans)
  ans = strcmp(a.getName().c_str(), b.getName().c_str());
 return ans;
}

