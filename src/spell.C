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

void BTSpell::activate(BTDisplay &d, const char *activation, bool partySpell, BTCombat *combat, int group, int target /*= BTTARGET_INDIVIDUAL*/)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 BTFactory<BTSpell> &spellList = game->getSpellList();
 int index = spellList.find(this);
 if (index >= spellList.size())
  return;
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
 switch(type)
 {
  case BTSPELLTYPE_HEAL:
   if (BTTARGET_PARTY == group)
   {
    if (text.length() > 0)
     text += " ";
    text += effect;
    if (BTTARGET_INDIVIDUAL == target)
    {
     text += " the whole party!";
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
     text += " ";
     text += party[target]->name;
     text += ".";
     if (party[target]->isAlive())
      party[target]->giveHP(dice.roll());
    }
    d.addText(text.c_str());
    d.addText("");
    d.process(BTDisplay::allKeys, 1000);
    d.clearElements();
    game->addEffect(index, expire, group, target);
    d.drawStats();
   }
   break;
  case BTSPELLTYPE_RESURRECT:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     if (text.length() > 0)
      text += " ";
     text += effect;
     text += " the whole party!";
    }
    d.addText(text.c_str());
    d.addText("");
    d.process(BTDisplay::allKeys, 1000);
    d.clearElements();
    if (BTTARGET_INDIVIDUAL == target)
    {
     game->addEffect(index, expire, group, target);
     for (int i = 0; i < party.size(); ++i)
     {
      if (!party[i]->isAlive())
      {
       text = party[i]->name;
       text += " rises from the dead!";
       d.addText(text.c_str());
       d.addText("");
       d.process(BTDisplay::allKeys, 1000);
       d.clearElements();
       party[i]->status.clear(BTSTATUS_DEAD);
       party[i]->hp = 1;
      }
     }
    }
    else
    {
     if (!party[target]->isAlive())
     {
      text = effect;
      text += " ";
      text += party[target]->name;
      text += ". ";
      text += party[target]->name;
      text += " rises from the dead!";
      party[target]->status.clear(BTSTATUS_DEAD);
      party[target]->hp = 1;
      d.addText(text.c_str());
      d.addText("");
      d.process(BTDisplay::allKeys, 1000);
     }
    }
    d.drawStats();
   }
   break;
  case BTSPELLTYPE_LIGHT:
   if (text.length() > 0)
    text += " ";
   text += effect;
   d.addText(text.c_str());
   d.addText("");
   d.process(BTDisplay::allKeys, 1000);
   game->addEffect(index, expire, group, target);
   break;
  case BTSPELLTYPE_SUMMONMONSTER:
  case BTSPELLTYPE_SUMMONILLUSION:
  {
   BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
   if (text.length() > 0)
    text += " ";
   text += effect;
   d.addText(text.c_str());
   d.addText("");
   d.process(BTDisplay::allKeys, 1000);
   if (party.size() >= BT_PARTYSIZE)
   {
    text = "No room in your party. ";
    text += monsterList[extra].getName();
    text += " cannot join!";
    d.clearElements();
    d.addText(text.c_str());
    d.addText("");
    d.process(BTDisplay::allKeys, 1000);
   }
   else
   {
    BTPc *pc = new BTPc;
    pc->setName(monsterList[extra].getName());
    pc->race = -1;
    pc->job = ((type == BTSPELLTYPE_SUMMONMONSTER) ? BTJOB_MONSTER : BTJOB_ILLUSION);
    pc->picture = monsterList[extra].getPicture();
    pc->monster = extra;
    pc->ac = monsterList[extra].getAc();
    pc->hp = pc->maxHp = monsterList[extra].getHp().roll();
    party.push_back(pc);
    d.drawStats();
    if ((BTTIME_PERMANENT != expire) && (BTTIME_CONTINUOUS != expire))
     game->addEffect(index, expire, BTTARGET_PARTY, party.size() - 1);
   }
   break;
  }
  default:
   break;
 }
 d.clearElements();
}

void BTSpell::cast(BTDisplay &d, const char *caster, bool partySpell, BTCombat *combat, int group, int target /*= BTTARGET_INDIVIDUAL*/)
{
 std::string text = caster;
 text += " casts ";
 text += name;
 text += ".";
 activate(d, text.c_str(), partySpell, combat, group, target);
}

void BTSpell::finish(BTDisplay &d, BTCombat *combat, int group, int target /*= BTTARGET_INDIVIDUAL*/)
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
     d.addText(text.c_str());
     d.addText("");
     d.process(BTDisplay::allKeys, 1000);
     d.clearElements();
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
     d.addText(text.c_str());
     d.addText("");
     d.process(BTDisplay::allKeys, 1000);
     d.clearElements();
    }
   }
   break;
  }
 }
}

void BTSpell::maintain(BTDisplay &d, BTCombat *combat, int group, int target /*= BTTARGET_INDIVIDUAL*/)
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
       d.addText(text.c_str());
       d.addText("");
       d.process(BTDisplay::allKeys, 1000);
       d.clearElements();
       party[i]->status.clear(BTSTATUS_DEAD);
       party[i]->hp = 1;
      }
     }
    }
    else
    {
     // BTCS either cancels spells on death or doesn't implement this
/*     if (!party[target]->isAlive())
     {
      text = effect;
      text += " ";
      text += party[target]->name;
      text += ". ";
      text += party[target]->name;
      text += " rises from the dead!";
      party[target]->status.clear(BTSTATUS_DEAD);
      party[target]->hp = 1;
      d.addText(text.c_str());
      d.addText("");
      d.process(BTDisplay::allKeys, 1000);
     }*/
    }
   }
   break;
  default:
   break;
 }
 d.clearElements();
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
