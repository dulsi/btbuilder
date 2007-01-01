/*-------------------------------------------------------------------------*\
  <item.C> -- Item implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "item.h"

BTItem::BTItem(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByteArray(25, (IUByte *)name);
 f.readUByte(unknown);
 f.readShort(timesUsable);
 damage.read(f);
 f.readUByte(unknown);
 f.readShort(armorPlus);
 f.readShort(hitPlus);
 f.readShort(xSpecial);
 f.readShort(chanceXSpecial);
 f.readShort(type);
 f.readShort(spellCast);
 f.readShort(classAllowed);
 f.readShort(price);
 f.readUByteArray(24, (IUByte *)cause);
 f.readUByteArray(24, (IUByte *)effect);
}

BTItem::BTItem()
{
}

const char *BTItem::getName() const
{
 return name;
}

IShort BTItem::getArmorPlus() const
{
 return armorPlus;
}

const char *BTItem::getCause() const
{
 return cause;
}

IShort BTItem::getChanceXSpecial() const
{
 return chanceXSpecial;
}

const BTDice &BTItem::getDamage() const
{
 return damage;
}

const char *BTItem::getEffect() const
{
 return effect;
}

IShort BTItem::getHitPlus() const
{
 return hitPlus;
}

IShort BTItem::getPrice() const
{
 return price;
}

IShort BTItem::getSpellCast() const
{
 return spellCast;
}

IShort BTItem::getTimesUsable() const
{
 return timesUsable;
}

IShort BTItem::getType() const
{
 return type;
}

IShort BTItem::getXSpecial() const
{
 return xSpecial;
}

void BTItem::write(BinaryWriteFile &f)
{
 IUByte unknown = 0x00;

 f.writeUByteArray(25, (IUByte *)name);
 f.writeUByte(unknown);
 f.writeShort(timesUsable);
 damage.write(f);
 f.writeUByte(unknown);
 f.writeShort(armorPlus);
 f.writeShort(hitPlus);
 f.writeShort(xSpecial);
 f.writeShort(chanceXSpecial);
 f.writeShort(type);
 f.writeShort(spellCast);
 f.writeShort(classAllowed);
 f.writeShort(price);
 f.writeUByteArray(24, (IUByte *)cause);
 f.writeUByteArray(24, (IUByte *)effect);
}

