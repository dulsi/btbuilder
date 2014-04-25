/*-------------------------------------------------------------------------*\
  <item.C> -- Item implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "item.h"
#include "pc.h"
#include "game.h"

int BTItem::compatJobAllowed[11] = {0x80, 0x04, 0x10, 0x08, 0x02, 0x01, 0x40, 0x40, 0x40, 0x20, 0x20};

BTItem::BTItem(BinaryReadFile &f)
{
 IUByte unknown;
 char tmp[26];
 IShort num;

 f.readUByteArray(25, (IUByte *)tmp);
 tmp[25] = 0;
 name = tmp;
 f.readUByte(unknown);
 f.readShort(timesUsable);
 damage.read(f);
 f.readUByte(unknown);
 f.readShort(armorPlus);
 f.readShort(hitPlus);
 f.readShort(num);
 xSpecial = num;
 f.readShort(chanceXSpecial);
 f.readShort(num);
 type = num;
 f.readShort(num);
 spellCast = num;
 IShort jobAllowed;
 f.readShort(jobAllowed);
 for (int i = 0; i < 11; i++)
 {
  if (jobAllowed & compatJobAllowed[i])
   classAllowed.set(i);
 }
 f.readShort(price);
 f.readUByteArray(24, (IUByte *)tmp);
 tmp[24] = 0;
 cause = new char[strlen(tmp) + 1];
 strcpy(cause, tmp);
 f.readUByteArray(24, (IUByte *)tmp);
 tmp[24] = 0;
 effect = new char[strlen(tmp) + 1];
 strcpy(effect, tmp);
 if ((BTITEM_ARROW == type) || (BTITEM_THROWNWEAPON == type))
  consume = true;
 else
  consume = false;
}

BTItem::BTItem()
 : timesUsable(0), armorPlus(0), hitPlus(0), xSpecial(0), chanceXSpecial(0), price(0), spellCast(0), type(0)
{
 cause = new char[1];
 cause[0] = 0;
 effect = new char[1];
 effect[0] = 0;
 consume = false;
}

BTItem::BTItem(const BTItem &copy)
 : name(copy.name), timesUsable(copy.timesUsable), damage(copy.damage), armorPlus(copy.armorPlus), hitPlus(copy.hitPlus),
 xSpecial(copy.xSpecial), chanceXSpecial(copy.chanceXSpecial), type(copy.type), spellCast(copy.spellCast),
 classAllowed(copy.classAllowed), price(copy.price), consume(copy.consume)
{
 cause = new char[strlen(copy.cause) + 1];
 strcpy(cause, copy.cause);
 effect = new char[strlen(copy.effect) + 1];
 strcpy(effect, copy.effect);
}

BTItem::~BTItem()
{
 if (cause)
  delete [] cause;
 if (effect)
  delete [] effect;
}

bool BTItem::canUse(BTPc *pc) const
{
 return classAllowed.isSet(pc->job);
}

const std::string &BTItem::getName() const
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

bool BTItem::isConsumed() const
{
 return consume;
}

void BTItem::write(BinaryWriteFile &f)
{
 IUByte unknown = 0x00;
 char tmp[25];
 IShort num;

 strncpy(tmp, name.c_str(), 25);
 f.writeUByteArray(25, (IUByte *)tmp);
 f.writeUByte(unknown);
 f.writeShort(timesUsable);
 damage.write(f);
 f.writeUByte(unknown);
 f.writeShort(armorPlus);
 f.writeShort(hitPlus);
 num = xSpecial;
 f.writeShort(num);
 f.writeShort(chanceXSpecial);
 num = type;
 f.writeShort(num);
 num = spellCast;
 f.writeShort(num);
 IShort jobAllowed(0);
 for (int i = 0; i < 11; i++)
 {
  if (classAllowed.isSet(i))
   jobAllowed |= compatJobAllowed[i];
 }
 f.writeShort(jobAllowed);
 f.writeShort(price);
 strncpy(tmp, cause, 24);
 f.writeUByteArray(24, (IUByte *)tmp);
 strncpy(tmp, effect, 24);
 f.writeUByteArray(24, (IUByte *)tmp);
}

void BTItem::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("timesUsable", &timesUsable);
 s->add("damage", &damage);
 s->add("armorPlus", &armorPlus);
 s->add("hitPlus", &hitPlus);
 s->add("xSpecial", &xSpecial, NULL, &extraDamageLookup);
 s->add("chanceXSpecial", &chanceXSpecial);
 s->add("type", &type, NULL, &itemTypesLookup);
 s->add("spellCast", &spellCast);
 s->add("spell", &spellCast, NULL, &BTCore::getCore()->getSpellList(), -1, "(none)");
 s->add("allowedJob", &classAllowed, &BTCore::getCore()->getJobList());
 s->add("price", &price);
 s->add("cause", &cause);
 s->add("effect", &effect);
 s->add("consume", &consume);
}

void BTItem::readXML(const char *filename, XMLVector<BTItem*> &item)
{
 XMLSerializer parser;
 parser.add("item", &item, &BTItem::create);
 parser.parse(filename, true);
}

void BTItem::writeXML(const char *filename, XMLVector<BTItem*> &item)
{
 XMLSerializer parser;
 parser.add("item", &item, &BTItem::create);
 parser.write(filename, true);
}

int BTItemListCompare::Compare(const BTItem &a, const BTItem &b) const
{
 return strcmp(a.getName().c_str(), b.getName().c_str());
}

