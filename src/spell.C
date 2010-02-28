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

 f.writeUByteArray(29, (IUByte *)name);
 f.writeUByteArray(5, (IUByte *)code);
 f.writeShort(caster);
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

int BTSpell::activate(BTDisplay &d, const char *activation, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target)
{
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 int index = spellList.find(this);
 if (index >= spellList.size())
  return 0;
 unsigned int expire = 0;
 switch(duration)
 {
  case BTDURATION_ONE:
   expire = game->getExpiration(1);
   break;
  case BTDURATION_SHORT:
   expire = game->getExpiration(BTDice(1, 121, 119).roll());
   break;
  case BTDURATION_MEDIUM:
   expire = game->getExpiration(BTDice(1, 181, 179).roll());
   break;
  case BTDURATION_LONG:
   expire = game->getExpiration(BTDice(1, 241, 239).roll());
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
 std::string text = activation;
 if (text.length() > 0)
  text += " ";
 text += effect;
 switch(area)
 {
  case BTAREAEFFECT_GROUP:
   if (BTTARGET_PARTY == group)
    text += " the whole party!";
   break;
  case BTAREAEFFECT_FOE:
   text += " ";
   if (BTTARGET_PARTY == group)
   {
    text += party[target]->name;
   }
   text += ".";
  case BTAREAEFFECT_NONE:
  default:
   break;
 }
 d.drawMessage(text.c_str(), game->getDelay());
 BTBaseEffect *effect = NULL;
 switch(type)
 {
  case BTSPELLTYPE_RESURRECT:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     effect = new BTResurrectEffect(type, expire, group, target);
    }
    else
    {
     if (!party[target]->isAlive())
     {
      text = party[target]->name;
      text += " rises from the dead!";
      party[target]->status.clear(BTSTATUS_DEAD);
      party[target]->hp = 1;
      d.drawMessage(text.c_str(), game->getDelay());
      d.drawStats();
     }
    }
   }
   break;
  case BTSPELLTYPE_SUMMONMONSTER:
  case BTSPELLTYPE_SUMMONILLUSION:
  {
   if (party.size() >= BT_PARTYSIZE)
   {
    BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
    text = "No room in your party. ";
    text += monsterList[extra].getName();
    text += " cannot join!";
    d.drawMessage(text.c_str(), game->getDelay());
   }
   else
   {
    BTPc *pc = new BTPc(extra, ((type == BTSPELLTYPE_SUMMONMONSTER) ? BTJOB_MONSTER : BTJOB_ILLUSION));
    party.add(d, pc);
    d.drawStats();
    if ((BTTIME_PERMANENT != expire) && (BTTIME_CONTINUOUS != expire))
    {
     if (type == BTSPELLTYPE_SUMMONMONSTER)
      effect = new BTSummonMonsterEffect(index, expire, BTTARGET_PARTY, party.size() - 1);
     else
      effect = new BTSummonIllusionEffect(index, expire, BTTARGET_PARTY, party.size() - 1);
    }
   }
   break;
  }
  case BTSPELLTYPE_PHASEDOOR:
  {
   int x = game->getX();
   int y = game->getY();
   int f = game->getFacing();
   for (int i = 0; i < 4; ++i)
   {
    int testX = (x + (Psuedo3D::changeXY[f][0] * i) + game->getMap()->getXSize()) % game->getMap()->getXSize();
    int testY = (y + (Psuedo3D::changeXY[f][1] * i) + game->getMap()->getYSize()) % game->getMap()->getYSize();
    if (game->getWallType(testX, testY, f))
    {
     effect = new BTPhaseDoorEffect(type, BTTIME_MAP, testX, testY, f);
     break;
    }
   }
   break;
  }
  case BTSPELLTYPE_LIGHT:
  case BTSPELLTYPE_DOORDETECT:
  case BTSPELLTYPE_TRAPDESTROY:
  case BTSPELLTYPE_COMPASS:
   effect = new BTBaseEffect(type, expire);
   break;
  case BTSPELLTYPE_DAMAGE:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, dice, BTSTATUS_NONE, "");
   break;
  case BTSPELLTYPE_KILL:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_DEAD, " is killed");
   break;
  case BTSPELLTYPE_POISON:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_POISONED, " is poisoned");
   break;
  case BTSPELLTYPE_CAUSEINSANITY:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_DEAD, " has gone insane");
   break;
  case BTSPELLTYPE_POSSESS:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_POSSESSED, "is possessed");
   break;
  case BTSPELLTYPE_FLESHTOSTONE:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_STONED, "is stoned");
   break;
  case BTSPELLTYPE_PARALYZE:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTSTATUS_PARALYZED, "is paralyzed");
   break;
  case BTSPELLTYPE_DAMAGEBYLEVEL:
   effect = new BTAttackEffect(type, expire, range, getEffectiveRange(), distance, group, target, BTDice(dice.getNumber() * casterLevel, dice.getType(), dice.getModifier()), BTSTATUS_NONE, "");
   break;
  case BTSPELLTYPE_CUREPOISON:
   effect = new BTCureStatusEffect(type, expire, group, target, BTSTATUS_POISONED);
   break;
  case BTSPELLTYPE_CUREINSANITY:
   effect = new BTCureStatusEffect(type, expire, group, target, BTSTATUS_INSANE);
   break;
  case BTSPELLTYPE_DISPOSSESS:
   effect = new BTCureStatusEffect(type, expire, group, target, BTSTATUS_POSSESSED);
   break;
  case BTSPELLTYPE_STONETOFLESH:
   effect = new BTCureStatusEffect(type, expire, group, target, BTSTATUS_STONED);
   break;
  case BTSPELLTYPE_CUREPARALYZE:
   effect = new BTCureStatusEffect(type, expire, group, target, BTSTATUS_PARALYZED);
   break;
  case BTSPELLTYPE_HEAL:
   effect = new BTHealEffect(type, expire, group, target, dice);
   break;
  case BTSPELLTYPE_ARMORBONUS:
   effect = new BTArmorBonusEffect(type, expire, group, target, getExtra());
   break;
  default:
   break;
 }
 if (effect)
 {
  try
  {
   killed = effect->apply(d, combat);
   game->addEffect(effect);
  }
  catch (const BTAllResistException &e)
  {
   delete effect;
  }
 }
 return killed;
}

int BTSpell::cast(BTDisplay &d, const char *caster, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target)
{
 std::string text = caster;
 text += " casts ";
 text += name;
 text += ".";
 return activate(d, text.c_str(), partySpell, combat, casterLevel, distance, group, target);
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
