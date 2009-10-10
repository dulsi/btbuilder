/*-------------------------------------------------------------------------*\
  <spell.C> -- Spell implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spell.h"
#include "game.h"

#define BTSPELLFLG_DAMAGE 0
#define BTSPELLFLG_KILLED 1
#define BTSPELLFLG_EXCLAMATION 2

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
 BitField resists;
 if (checkResists(combat, group, target, resists))
 {
  displayResists(d, combat, group, target);
  return 0;
 }
 switch(type)
 {
  case BTSPELLTYPE_RESURRECT:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     game->addEffect(index, expire, casterLevel, distance, group, target, resists);
     for (int i = 0; i < party.size(); ++i)
     {
      if (!party[i]->isAlive())
      {
       text = party[i]->name;
       text += " rises from the dead!";
       d.drawMessage(text.c_str(), game->getDelay());
       party[i]->status.clear(BTSTATUS_DEAD);
       party[i]->hp = 1;
      }
     }
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
     }
    }
    d.drawStats();
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
     game->addEffect(index, expire, casterLevel, distance, BTTARGET_PARTY, party.size() - 1, resists);
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
     game->addEffect(index, BTTIME_MAP, casterLevel, distance, BTTARGET_PARTY, ((testY * game->getMap()->getXSize()) + testX) * BT_DIRECTIONS + f, resists);
     break;
    }
   }
   break;
  }
  default:
   killed = apply(d, partySpell, combat, casterLevel, distance, group, target, resists);
   game->addEffect(index, expire, casterLevel, distance, group, target, resists);
   break;
 }
 return killed;
}

int BTSpell::apply(BTDisplay &d, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target, BitField &resists)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 switch(type)
 {
  case BTSPELLTYPE_HEAL:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < party.size(); ++i)
     {
      if (party[i]->isAlive())
      {
       party[i]->giveHP(dice.roll());
      }
     }
    }
    else
    {
     if (party[target]->isAlive())
      party[target]->giveHP(dice.roll());
    }
    d.drawStats();
   }
   break;
  case BTSPELLTYPE_CUREPOISON:
   cureStatus(combat, group, target, BTSTATUS_POISONED);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_CUREINSANITY:
   cureStatus(combat, group, target, BTSTATUS_INSANE);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_DISPOSSESS:
   cureStatus(combat, group, target, BTSTATUS_POSSESSED);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_DAMAGE:
  {
   int killed = setStatus(d, combat, distance, group, target, resists, dice, BTSTATUS_NONE, "", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   return killed;
  }
  case BTSPELLTYPE_KILL:
  {
   int killed = setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_DEAD, " is killed", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   return killed;
  }
  case BTSPELLTYPE_POISON:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_POISONED, " is poisoned", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_CAUSEINSANITY:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_INSANE, " has gone insane", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_POSSESS:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_POSSESSED, " is possessed", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_PARALYZE:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_PARALYZED, " is paralyzed", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_LIGHT:
  case BTSPELLTYPE_DOORDETECT:
   break;
  case BTSPELLTYPE_ARMORBONUS:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < party.size(); ++i)
     {
      party[i]->ac += getExtra();
     }
    }
    else
    {
     party[target]->ac += getExtra();
    }
    d.drawStats();
   }
   else if (BTTARGET_ALLMONSTERS == group)
   {
    for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
    {
     BTMonsterGroup *grp = combat->getMonsterGroup(i);
     if (NULL == grp)
      break;
     for (int k = 0; k < grp->individual.size(); ++k)
     {
      grp->individual[k].ac += getExtra();
     }
    }
   }
   else if (group >= BTTARGET_MONSTER)
   {
    BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < grp->individual.size(); ++i)
     {
      grp->individual[i].ac += getExtra();
     }
    }
    else
    {
     grp->individual[target].ac += getExtra();
    }
   }
   break;
  case BTSPELLTYPE_DAMAGEBYLEVEL:
  {
   int killed = setStatus(d, combat, distance, group, target, resists, BTDice(dice.getNumber() * casterLevel, dice.getType(), dice.getModifier()), BTSTATUS_NONE, "", true);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   return killed;
  }
  default:
   break;
 }
 return 0;
}

int BTSpell::cast(BTDisplay &d, const char *caster, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target)
{
 std::string text = caster;
 text += " casts ";
 text += name;
 text += ".";
 return activate(d, text.c_str(), partySpell, combat, casterLevel, distance, group, target);
}

void BTSpell::finish(BTDisplay &d, BTCombat *combat, int casterLevel, int distance, int group, int target, BitField &resists)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 switch(type)
 {
  case BTSPELLTYPE_SUMMONMONSTER:
  {
   if (BTTARGET_PARTY == group)
   {
    std::string text = party[target]->name;
    if (party.remove(target, d))
    {
     text += " leaves your party.";
     d.drawMessage(text.c_str(), game->getDelay());
    }
   }
   break;
  }
  case BTSPELLTYPE_SUMMONILLUSION:
  {
   if (BTTARGET_PARTY == group)
   {
    std::string text = "The illusionary ";
    text += party[target]->name;
    if (party.remove(target, d))
    {
     text += " disappears!";
     d.drawMessage(text.c_str(), game->getDelay());
    }
   }
   break;
  }
  case BTSPELLTYPE_POISON:
   cureStatus(combat, group, target, BTSTATUS_POISONED);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_CAUSEINSANITY:
   cureStatus(combat, group, target, BTSTATUS_INSANE);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_POSSESS:
   cureStatus(combat, group, target, BTSTATUS_POSSESSED);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_PARALYZE:
   cureStatus(combat, group, target, BTSTATUS_PARALYZED);
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_ARMORBONUS:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < party.size(); ++i)
     {
      party[i]->ac -= getExtra();
     }
    }
    else
    {
     party[target]->ac -= getExtra();
    }
    d.drawStats();
   }
   else if (BTTARGET_ALLMONSTERS == group)
   {
    for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
    {
     BTMonsterGroup *grp = combat->getMonsterGroup(i);
     if (NULL == grp)
      break;
     for (int k = 0; k < grp->individual.size(); ++k)
     {
      grp->individual[k].ac -= getExtra();
     }
    }
   }
   else if (group >= BTTARGET_MONSTER)
   {
    BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < grp->individual.size(); ++i)
     {
      grp->individual[i].ac -= getExtra();
     }
    }
    else
    {
     grp->individual[target].ac -= getExtra();
    }
   }
   break;
  default:
   break;
 }
}

void BTSpell::maintain(BTDisplay &d, BTCombat *combat, int casterLevel, int distance, int group, int target, BitField &resists)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 switch(type)
 {
  case BTSPELLTYPE_HEAL:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < party.size(); ++i)
     {
      if (party[i]->isAlive())
      {
       party[i]->giveHP(dice.roll());
      }
     }
    }
    else
    {
     if (party[target]->isAlive())
      party[target]->giveHP(dice.roll());
    }
    d.drawStats();
   }
   break;
  case BTSPELLTYPE_RESURRECT:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     for (int i = 0; i < party.size(); ++i)
     {
      if (!party[i]->isAlive())
      {
       std::string text = party[i]->name;
       text += " rises from the dead!";
       d.drawMessage(text.c_str(), game->getDelay());
       party[i]->status.clear(BTSTATUS_DEAD);
       party[i]->hp = 1;
      }
     }
    }
    else
    {
     // BTCS either cancels spells on death or doesn't implement this
    }
   }
   break;
  case BTSPELLTYPE_CUREPOISON:
   cureStatus(combat, group, target, BTSTATUS_POISONED);
   d.drawStats();
   break;
  case BTSPELLTYPE_CUREINSANITY:
   cureStatus(combat, group, target, BTSTATUS_INSANE);
   d.drawStats();
   break;
  case BTSPELLTYPE_DISPOSSESS:
   cureStatus(combat, group, target, BTSTATUS_POSSESSED);
   d.drawStats();
   break;
  case BTSPELLTYPE_DAMAGE:
   setStatus(d, combat, distance, group, target, resists, dice, BTSTATUS_NONE, "");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_KILL:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_DEAD, " is killed.");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_POISON:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_POISONED, " is poisoned.");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_CAUSEINSANITY:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_INSANE, " has gone insane.");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_POSSESS:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_POSSESSED, " is possessed.");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_PARALYZE:
   setStatus(d, combat, distance, group, target, resists, BTDice(0, 2), BTSTATUS_PARALYZED, " is paralyzed.");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  case BTSPELLTYPE_DAMAGEBYLEVEL:
   setStatus(d, combat, distance, group, target, resists, BTDice(dice.getNumber() * casterLevel, dice.getType(), dice.getModifier()), BTSTATUS_NONE, "");
   if (BTTARGET_PARTY == group)
    d.drawStats();
   break;
  default:
   break;
 }
 d.clearElements();
}

bool BTSpell::checkResists(BTCombat *combat, int group, int target, BitField &resists)
{
 switch(type)
 {
  case BTSPELLTYPE_KILL:
  case BTSPELLTYPE_POISON:
  case BTSPELLTYPE_CAUSEINSANITY:
  case BTSPELLTYPE_POSSESS:
  case BTSPELLTYPE_PARALYZE:
   break;
  default:
   return false;
   break;
 }
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 if (BTTARGET_ALLMONSTERS == group)
 {
  int total = 0;
  bool allResists = true;
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   int resistance = monList[grp->monsterType].getMagicResistance();
   if (resistance > 0)
   {
    for (int k = 0; k < grp->individual.size(); ++k)
    {
     if (BTDice(1, 100).roll() > resistance)
      resists.set(total + k);
     else
      allResists = false;
    }
   }
   else
    allResists = false;
   total = grp->individual.size();
  }
  if (allResists)
   return true;
 }
 else if (BTTARGET_PARTY == group)
 {
  BTParty &party = game->getParty();
  if (BTTARGET_INDIVIDUAL == target)
  {
   bool allResists = true;
   for (int i = 0; i < party.size(); ++i)
   {
    if (BTMONSTER_NONE != party[i]->monster)
    {
     if (BTDice(1, 100).roll() > monList[party[i]->monster].getMagicResistance())
      resists.set(i);
     else
      allResists = false;
    }
    else
     allResists = false;
   }
   if (allResists)
    return true;
  }
  else
  {
   if (BTMONSTER_NONE != party[target]->monster)
   {
    if (BTDice(1, 100).roll() > monList[party[target]->monster].getMagicResistance())
    {
     resists.set(0);
     return true;
    }
   }
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  int resistance = monList[grp->monsterType].getMagicResistance();
  if (resistance > 0)
  {
   if (BTTARGET_INDIVIDUAL == target)
   {
    bool allResists = true;
    for (int i = 0; i < grp->individual.size(); ++i)
    {
     if (BTDice(1, 100).roll() > resistance)
      resists.set(i);
     else
      allResists = false;
    }
    if (allResists)
     return true;
   }
   else if (BTDice(1, 100).roll() > resistance)
   {
    resists.set(0);
    return true;
   }
  }
 }
 return false;
}

void BTSpell::cureStatus(BTCombat *combat, int group, int target, int status)
{
 if (BTTARGET_PARTY == group)
 {
  BTGame *game = BTGame::getGame();
  BTParty &party = game->getParty();
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->status.isSet(status))
    {
     party[i]->status.clear(status);
    }
   }
  }
  else
  {
   if (party[target]->status.isSet(status))
   {
    party[target]->status.clear(status);
   }
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    if ((grp->individual[k].isAlive()) && (!grp->individual[k].status.isSet(status)))
    {
     grp->individual[k].status.clear(status);
    }
   }
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    if (grp->individual[i].status.isSet(status))
    {
     grp->individual[i].status.clear(status);
    }
   }
  }
  else
  {
   if (grp->individual[target].status.isSet(status))
   {
    grp->individual[target].status.clear(status);
   }
  }
 }
}

void BTSpell::displayResists(BTDisplay &d, BTCombat *combat, int group, int target)
{
 std::string text;
 if (BTTARGET_ALLMONSTERS == group)
 {
  text = "All monsters";
 }
 else if (BTTARGET_PARTY == group)
 {
  BTParty &party = BTGame::getGame()->getParty();
  if (BTTARGET_INDIVIDUAL == target)
   text = "The whole party";
  else
   text = party[target]->name;
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTFactory<BTMonster> &monList = BTGame::getGame()->getMonsterList();
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  text = monList[grp->monsterType].getName();
  if ((BTTARGET_INDIVIDUAL == target) && (1 < grp->individual.size()))
   text += "(s)";
 }
 text += " repelled the spell!";
 d.drawMessage(text.c_str(), BTGame::getGame()->getDelay());
}

int BTSpell::setStatus(BTDisplay &d, BTCombat *combat, int distance, int group, int target, BitField &resists, const BTDice &dam, int status, const char *statusText, bool first /*= false*/)
{
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 if (BTTARGET_PARTY == group)
 {
  BTParty &party = game->getParty();
  if (distance > (range * (1 + getEffectiveRange())))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < party.size(); ++i)
   {
    int damage = dam.roll();
    if (distance > range)
     damage = damage / 2;
    BitField flags;
    if ((party[i]->isAlive()) && (resists.isSet(i)))
    {
     if (first)
     {
      damage = damage / 2;
      if (party[i]->takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(party[i]->name, ((damage > 0) ? "saves and takes" : "repealed the spell"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (party[i]->active))
      {
       ++killed;
       party[i]->active = false;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
    else if ((party[i]->isAlive()) && (!party[i]->status.isSet(status)))
    {
     if (party[i]->savingThrow())
     {
      damage = damage / 2;
      if (party[i]->takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(party[i]->name, ((damage > 0) ? "saves and takes" : "saves"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (party[i]->active))
      {
       ++killed;
       party[i]->active = false;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
     else
     {
      if (dam.getNumber())
       flags.set(BTSPELLFLG_DAMAGE);
      if (party[i]->takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      std::string text = message(party[i]->name, ((damage > 0) ? "takes" : ""), damage, statusText, flags);
      d.drawMessage(text.c_str(), game->getDelay());
      if (status != BTSTATUS_NONE)
       party[i]->status.set(status);
      if ((!party[i]->isAlive()) && (party[i]->active))
      {
       ++killed;
       party[i]->active = false;
      }
     }
    }
   }
  }
  else
  {
   if ((party[target]->isAlive()) && (!party[target]->status.isSet(status)))
   {
    int damage = dam.roll();
    if (distance > range)
     damage = damage / 2;
    BitField flags;
    if (party[target]->savingThrow())
    {
     damage = damage / 2;
     if (party[target]->takeHP(damage))
     {
      flags.set(BTSPELLFLG_KILLED);
      flags.set(BTSPELLFLG_EXCLAMATION);
     }
     if (damage == 0)
      flags.set(BTSPELLFLG_EXCLAMATION);
     std::string text = message(party[target]->name, ((damage > 0) ? "saves and takes" : "saves!"), damage, "", flags);
     if ((flags.isSet(BTSPELLFLG_KILLED)) && (party[target]->active))
     {
      ++killed;
      party[target]->active = false;
     }
     d.drawMessage(text.c_str(), game->getDelay());
    }
    else
    {
     if (dam.getNumber())
      flags.set(BTSPELLFLG_DAMAGE);
     if (party[target]->takeHP(damage))
     {
      flags.set(BTSPELLFLG_KILLED);
      flags.set(BTSPELLFLG_EXCLAMATION);
     }
     std::string text = message(party[target]->name, ((damage > 0) ? "takes" : ""), damage, statusText, flags);
     d.drawMessage(text.c_str(), game->getDelay());
     if (status != BTSTATUS_NONE)
      party[target]->status.set(status);
     if ((!party[target]->isAlive()) && (party[target]->active))
     {
      ++killed;
      party[target]->active = false;
     }
    }
   }
  }
 }
 else if (BTTARGET_ALLMONSTERS == group)
 {
  for (int i = 0; i < BTCOMBAT_MAXENCOUNTERS; ++i)
  {
   BTMonsterGroup *grp = combat->getMonsterGroup(i);
   if (NULL == grp)
    break;
   if (abs(grp->distance - distance) > (range * (1 + getEffectiveRange())))
    continue;
   for (int k = 0; k < grp->individual.size(); ++k)
   {
    int damage = dam.roll();
    if (abs(grp->distance - distance) > range)
     damage = damage / 2;
    BitField flags;
    if ((grp->individual[k].isAlive()) && (resists.isSet(k)))
    {
     if (first)
     {
      damage = damage / 2;
      if (grp->individual[k].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "saves and takes" : "repelled the spell"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (grp->individual[k].active))
      {
       ++killed;
       grp->individual[k].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
    else if ((grp->individual[k].isAlive()) && (!grp->individual[k].status.isSet(status)))
    {
     if (monList[grp->monsterType].savingThrow())
     {
      damage = damage / 2;
      if (grp->individual[k].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "saves and takes" : "saves"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (grp->individual[k].active))
      {
       ++killed;
       grp->individual[k].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
     else
     {
      if (dam.getNumber())
       flags.set(BTSPELLFLG_DAMAGE);
      if (grp->individual[k].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "takes" : ""), damage, statusText, flags);
      if (status != BTSTATUS_NONE)
       grp->individual[k].status.set(status);
      if ((!grp->individual[k].isAlive()) && (grp->individual[k].active))
      {
       ++killed;
       grp->individual[k].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
   }
  }
 }
 else if (group >= BTTARGET_MONSTER)
 {
  BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
  if (abs(grp->distance - distance) > (range * (1 + getEffectiveRange())))
   return 0;
  if (BTTARGET_INDIVIDUAL == target)
  {
   for (int i = 0; i < grp->individual.size(); ++i)
   {
    int damage = dam.roll();
    if (abs(grp->distance - distance) > range)
     damage = damage / 2;
    BitField flags;
    if ((grp->individual[i].isAlive()) && (resists.isSet(i)))
    {
     if (first)
     {
      damage = damage / 2;
      if (grp->individual[i].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "saves and takes" : "repelled the spell"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (grp->individual[i].active))
      {
       ++killed;
       grp->individual[i].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
    else if ((grp->individual[i].isAlive()) && (!grp->individual[i].status.isSet(status)))
    {
     if (monList[grp->monsterType].savingThrow())
     {
      damage = damage / 2;
      if (grp->individual[i].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      if (damage == 0)
       flags.set(BTSPELLFLG_EXCLAMATION);
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "saves and takes" : "saves"), damage, "", flags);
      if ((flags.isSet(BTSPELLFLG_KILLED)) && (grp->individual[i].active))
      {
       ++killed;
       grp->individual[i].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
     else
     {
      if (dam.getNumber())
       flags.set(BTSPELLFLG_DAMAGE);
      if (grp->individual[i].takeHP(damage))
      {
       flags.set(BTSPELLFLG_KILLED);
       flags.set(BTSPELLFLG_EXCLAMATION);
      }
      std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "takes" : ""), damage, statusText, flags);
      if (status != BTSTATUS_NONE)
       grp->individual[i].status.set(status);
      if ((!grp->individual[i].isAlive()) && (grp->individual[i].active))
      {
       ++killed;
       grp->individual[i].active = false;
       grp->active--;
      }
      d.drawMessage(text.c_str(), game->getDelay());
     }
    }
   }
  }
  else
  {
   if ((grp->individual[target].isAlive()) && (!grp->individual[target].status.isSet(status)))
   {
    int damage = dam.roll();
    if (abs(grp->distance - distance) > range)
     damage = damage / 2;
    BitField flags;
    if (monList[grp->monsterType].savingThrow())
    {
     damage = damage / 2;
     if (grp->individual[target].takeHP(damage))
     {
      flags.set(BTSPELLFLG_KILLED);
      flags.set(BTSPELLFLG_EXCLAMATION);
     }
     if (damage == 0)
      flags.set(BTSPELLFLG_EXCLAMATION);
     std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "saves and takes" : "saves"), damage, "", flags);
     if ((flags.isSet(BTSPELLFLG_KILLED)) && (grp->individual[target].active))
     {
      ++killed;
      grp->individual[target].active = false;
      grp->active--;
     }
     d.drawMessage(text.c_str(), game->getDelay());
    }
    else
    {
     if (dam.getNumber())
      flags.set(BTSPELLFLG_DAMAGE);
     if (grp->individual[target].takeHP(damage))
     {
      flags.set(BTSPELLFLG_KILLED);
      flags.set(BTSPELLFLG_EXCLAMATION);
     }
     std::string text = message(monList[grp->monsterType].getName(), ((damage > 0) ? "takes" : ""), damage, statusText, flags);
     if (status != BTSTATUS_NONE)
      grp->individual[target].status.set(status);
     if ((!grp->individual[target].isAlive()) && (grp->individual[target].active))
     {
      ++killed;
      grp->individual[target].active = false;
      grp->active--;
     }
     d.drawMessage(text.c_str(), game->getDelay());
    }
   }
  }
 }
 return killed;
}

std::string BTSpell::message(const char *name, const char *text, int damage, const char *status, const BitField &flags)
{
 std::string msg = name;
 if ((msg.length() > 0) && (text[0] != 0))
  msg += " ";
 msg += text;
 if ((damage > 0 ) || (flags.isSet(BTSPELLFLG_DAMAGE)))
 {
  if (msg.length() > 0)
   msg += " ";
  char tmp[20];
  sprintf(tmp, "%d", damage);
  msg += tmp;
  msg += " points of damage";
  if (flags.isSet(BTSPELLFLG_KILLED))
   msg += ", killing him";
  if (status[0] != 0)
   msg += " and";
 }
 if ((msg.length() > 0) && (status[0] != 0))
  msg += " ";
 msg += status;
 if (flags.isSet(BTSPELLFLG_EXCLAMATION))
  msg += "!";
 else
  msg += ".";
 return msg;
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
