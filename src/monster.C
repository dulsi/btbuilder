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
 IShort calcAc;
 f.readShort(calcAc);
 ac = (calcAc * -1) + 10;
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
 return rateAttacks * meleeDamage.getMax() + ac + hp.getMax();
}

IBool BTMonster::isIllusion() const
{
 return illusion;
}

void BTMonster::write(BinaryWriteFile &f)
{
 IUByte unknown;
 IShort value;

 f.writeUByteArray(14, (IUByte *)name);
 value = startDistance * 16;
 f.writeShort(value);
 f.writeShort(move);
 f.writeShort(rateAttacks);
 f.writeShort(illusion);
 f.writeShort(picture);
 f.writeShortArray(4, combatAction);
 f.writeShort(meleeExtra);
 IShort calcAc;
 calcAc = (ac - 10) * -1;
 f.writeShort(calcAc);
 f.writeShort(maxAppearing);
 hp.write(f);
 meleeDamage.write(f);
 f.writeUByteArray(14, (IUByte *)meleeMessage);
 f.writeShort(magicResistance);
 rangedDamage.write(f);
 f.writeUByte(unknown);
 f.writeShort(rangedExtra);
 f.writeShort(range);
 f.writeUByteArray(14, (IUByte *)rangedMessage);
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

