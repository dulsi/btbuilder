/*-------------------------------------------------------------------------*\
  <spell.C> -- Spell implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spell.h"
#include "game.h"

BTSpell::BTSpell(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByteArray(29, (IUByte *)name);
 f.readUByteArray(5, (IUByte *)code);
 f.readShort(caster);
 caster += 6;
 f.readShort(level);
 f.readShort(sp);
 f.readShort(range);
 f.readShort(effectiveRange);
 f.readShort(type);
 f.readShort(area);
 dice.read(f);
 f.readUByte(unknown);
 f.readShort(duration);
 f.readShort(extra);
 f.readUByteArray(22, (IUByte *)effect);
}

BTSpell::BTSpell()
{
}

const char *BTSpell::getName() const
{
 return name;
}

IShort BTSpell::getArea() const
{
 return area;
}

IShort BTSpell::getCaster() const
{
 return caster;
}

const char *BTSpell::getCode() const
{
 return code;
}

const BTDice &BTSpell::getDice() const
{
 return dice;
}

IShort BTSpell::getDuration() const
{
 return duration;
}

const char *BTSpell::getEffect() const
{
 return effect;
}

IShort BTSpell::getEffectiveRange() const
{
 return effectiveRange;
}

IShort BTSpell::getExtra() const
{
 return extra;
}

IShort BTSpell::getLevel() const
{
 return level;
}

IShort BTSpell::getRange() const
{
 return range;
}

IShort BTSpell::getSp() const
{
 return sp;
}

IShort BTSpell::getType() const
{
 return type;
}

void BTSpell::write(BinaryWriteFile &f)
{
 IUByte unknown = 0x00;
 IShort casterReal;

 f.writeUByteArray(29, (IUByte *)name);
 f.writeUByteArray(5, (IUByte *)code);
 casterReal = caster - 6;
 f.writeShort(casterReal);
 f.writeShort(level);
 f.writeShort(sp);
 f.writeShort(range);
 f.writeShort(effectiveRange);
 f.writeShort(type);
 f.writeShort(area);
 dice.write(f);
 f.writeUByte(unknown);
 f.writeShort(duration);
 f.writeShort(extra);
 f.writeUByteArray(22, (IUByte *)effect);
}

void BTSpell::cast(BTDisplay &d, const char *caster, BTCombat *combat, int group, int target /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 unsigned int expire = 0;
 switch(duration)
 {
  case BTDURATION_ONE:
   expire = game->getExpiration(1);
   break;
  case BTDURATION_SHORT:
   expire = game->getExpiration(BTDice(1, 21, 19).roll());
   break;
  case BTDURATION_MEDIUM:
   expire = game->getExpiration(BTDice(1, 31, 29).roll());
   break;
  case BTDURATION_LONG:
   expire = game->getExpiration(BTDice(1, 41, 39).roll());
   break;
  case BTDURATION_COMBAT:
   expire = BTTIME_COMBAT;
   break;
  case BTDURATION_PERMANENT:
   expire = BTTIME_PERMANENT;
   break;
  case BTDURATION_CONTINUOUS:
   expire = BTTIME_CONTINUOUS;
   break;
  case BTDURATION_INDEFINITE:
   expire = BTTIME_INDEFINITE;
   break;
  default:
   break;
 }
 switch(type)
 {
  case BTSPELLTYPE_LIGHT:
   game->addEffect(this, expire);
   break;
  default:
   break;
 }
}

int BTSpellListCompare::Compare(const BTSpell &a, const BTSpell &b) const
{
 int ans = a.getCaster() - b.getCaster();
 if (0 == ans)
 {
  ans = a.getLevel() - b.getLevel();
 }
 return ans;
}

BTSpellEffect::BTSpellEffect(int s, int x)
 : spell(s), expiration(x)
{
}

void BTSpellEffect::serialize(ObjectSerializer *s)
{
 s->add("spell", &spell);
 s->add("expiration", &expiration);
}
