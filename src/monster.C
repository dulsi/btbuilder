/*-------------------------------------------------------------------------*\
  <monster.C> -- Monster implementation file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "monster.h"

#define MONSTER_RANGEDTYPENONE   0
#define MONSTER_RANGEDTYPEONEFOE 1
#define MONSTER_RANGEDTYPEGROUP  2
#define MONSTER_RANGEDTYPEMAGIC  3

BTMonster::BTMonster(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByteArray(14, (IUByte *)name);
 f.readShort(startDistance);
 startDistance = startDistance / 16;
 f.readShort(move);
 f.readShort(rateAttacks);
 f.readShort(illusion);
 f.readShort(picture);
 f.readShortArray(4, combatAction);
 f.readShort(meleeExtra);
 f.readShort(ac);
 f.readShort(maxAppearing);
 hp.read(f);
 meleeDamage.read(f);
 f.readUByteArray(14, (IUByte *)meleeMessage);
 f.readShort(magicResistance);
 rangedDamage.read(f);
 f.readUByte(unknown);
 f.readShort(rangedExtra);
 f.readShort(range);
 f.readUByteArray(14, (IUByte *)rangedMessage);
 f.readShort(rangedType);
 if (rangedType >= MONSTER_RANGEDTYPEMAGIC)
 {
  rangedSpell = rangedType - MONSTER_RANGEDTYPEMAGIC;
  rangedType = MONSTER_RANGEDTYPEMAGIC;
 }
 f.readShort(level);
 gold.read(f);
 f.readUByte(unknown);
}

BTMonster::BTMonster()
{
 name[0] = 0;
 meleeMessage[0] = 0;
 rangedMessage[0] = 0;
}

const char *BTMonster::getName() const
{
 return name;
}

IShort BTMonster::getAc() const
{
 return ac;
}

IShort BTMonster::getCombatAction(IShort round) const
{
 return combatAction[round];
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
 return picture;
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

IBool BTMonster::isIllusion() const
{
 return illusion;
}

