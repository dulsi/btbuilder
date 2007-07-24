/*-------------------------------------------------------------------------*\
  <map.C> -- Map implementation file

  Date      Programmer  Description
  11/12/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "screenset.h"
#include "map.h"
#include "game.h"
#include "pc.h"
#include "ikbbuffer.h"

BTMapSquare::BTMapSquare()
 : wallInfo(0), special(-1)
{
}

IShort BTMapSquare::getWall(IShort dir) const
{
 return ((wallInfo >> (dir * 2)) & 0x03);
}

IShort BTMapSquare::getSpecial() const
{
 return special;
}

void BTMapSquare::read(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByte(wallInfo);
 f.readUByte(unknown);
 f.readShort(special);
}

void BTMapSquare::setSpecial(IShort s)
{
 special = s;
}

BTSpecialCommand::BTSpecialCommand()
{
 type = 0;
}

IShort BTSpecialCommand::getType() const
{
 return type;
}

void BTSpecialCommand::print(FILE *f) const
{
 char *dollarSign;
 char *start;
 long len;
 int count;

 count = 0;
 start = specialCommands[type];
 while (dollarSign = strchr(start, '$'))
 {
  len = (long)dollarSign - (long)start;
  fwrite(start, 1, len, f);
  switch (dollarSign[1])
  {
   case 'S':
    fprintf(f, "%s", BTGame::getGame()->getMap()->getSpecial(number[count++])->getName());
    break;
   case 'I':
    fprintf(f, "%s", BTGame::getGame()->getItemList()[number[count++]].getName());
    break;
   case 'A':
   case 'M':
    fprintf(f, "%s", BTGame::getGame()->getMonsterList()[number[count++]].getName());
    break;
   case 'X':
    fprintf(f, "%s", BTGame::getGame()->getSpellList()[number[count++]].getName());
    break;
   case 'L':
    fprintf(f, "X:%d Y:%d", number[count], number[count + 1]);
    count += 2;
    break;
   case 'T':
    fprintf(f, "%s", extraDamage[number[count++]]);
    break;
   case 'C':
    fprintf(f, "%s", BTGame::getGame()->getJobList()[number[count++]]->name);
    break;
   case 'R':
    fprintf(f, "%s", BTGame::getGame()->getRaceList()[number[count++]]->name);
    break;
   case 'D':
    fprintf(f, "%s", directions[number[count++]]);
    break;
   case '#':
   case 'P':
   case 'G':
   case 'F':
   case '!':
   case 'J':
    fprintf(f, "%d", number[count++]);
    break;
   case '$':
   case 'N':
   default:
    fprintf(f, "%s", text);
    break;
  }
  start = dollarSign + 2;
 }
 fprintf(f, "%s\n", start);
}

void BTSpecialCommand::read(BinaryReadFile &f)
{
 f.readShort(type);
 f.readUByteArray(26, (IUByte *)text);
 f.readShortArray(3, (IShort *)number);
}

void BTSpecialCommand::run(BTDisplay &d) const
{
 BTGame *game = BTGame::getGame();
 switch (type)
 {
  case BTSPECIALCOMMAND_STOP:
   throw BTSpecialStop();
   break;
  case BTSPECIALCOMMAND_SPIN:
   game->setFacing(BTDice(1, 4, 0).roll() % 4);
   break;
  case BTSPECIALCOMMAND_GETINPUT:
   game->setLastInput(d.readString("", 13));
   break;
  case BTSPECIALCOMMAND_TAKEITEM:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->takeItem(number[0]))
     break;
   }
   break;
  }
  case BTSPECIALCOMMAND_GIVEITEM:
  {
   XMLVector<BTPc*> &party = game->getParty();
   int who = 0;
   int charges = game->getItemList()[number[0]].getTimesUsable();
   for (; who < party.size(); ++who)
   {
    if (party[who]->giveItem(number[0], true, charges))
     break;
   }
   char tmp[100];
   if (who < party.size())
   {
    snprintf(tmp, 100, "%s gets %s.", party[who]->name, game->getItemList()[number[0]].getName());
   }
   else
   {
    snprintf(tmp, 100, "No one has room for %s!", game->getItemList()[number[0]].getName());
   }
   d.drawText(tmp);
   break;
  }
  case BTSPECIALCOMMAND_SELLITEM:
   d.drawText("");
   d.drawText("Pick a party member:");
   while (true)
   {
    char key = IKeybufferGet();
    if (27 == key)
     break;
    else if (('1' <= key) && ('9' >= key))
    {
     XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
     BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
     int n =  key - '1';
     if (n < party.size())
     {
      char tmp[100];
      d.drawText(party[n]->name);
      snprintf(tmp, 100, "%s costs %d gold coins.", itemList[number[0]].getName(), number[1]);
      d.drawText(tmp);
      d.drawText("Wilt thou pay?");
      d.drawText("Yes, or");
      d.drawText("No");
      while (true)
      {
       key = IKeybufferGet();
       if ((27 == key) || ('Y' == key) || ('y' == key) || ('N' == key) || ('n' == key))
        break;
      }
      if (('Y' == key) || ('y' == key))
      {
       if (party[n]->getGold() < number[1])
        snprintf(tmp, 100, "%s does not have enough gold!", party[n]->name);
       else if (party[n]->isEquipmentFull())
       {
        snprintf(tmp, 100, "%s has no room for %s!", party[n]->name, itemList[number[0]].getName());
        d.drawText(tmp);
        snprintf(tmp, 100, "%s does not have room to carry it!", party[n]->name);
       }
       else
       {
        party[n]->takeGold(number[1]);
        party[n]->giveItem(number[0], true, itemList[number[0]].getTimesUsable());
        snprintf(tmp, 100, "%s gets %s.", party[n]->name, itemList[number[0]].getName());
       }
       d.drawText(tmp);
      }
      break;
     }
    }
   }
   break;
  case BTSPECIALCOMMAND_PRINT:
   d.drawText(text);
   break;
  case BTSPECIALCOMMAND_REGENERATESPELLS:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->sp < party[who]->maxSp)
    {
     if (party[who]->sp + number[0] < party[who]->maxSp)
      party[who]->sp += number[0];
     else
      party[who]->sp = party[who]->maxSp;
    }
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_TAKESPELLS:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->sp > 0)
    {
     if (party[who]->sp < number[0])
      party[who]->sp = 0;
     else
      party[who]->sp -= number[0];
    }
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_HEALHITPOINTS:
  {
   BTParty &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    // BTCS heals even if dead.  Not sure if we want to mimic that
    if (party[who]->status.isSet(BTSTATUS_DEAD))
    {
     break;
    }
    party[who]->giveHP(number[0]);
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_DAMAGEHITPOINTS:
  {
   BTParty &party = game->getParty();
   char tmp[100];
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->status.isSet(BTSTATUS_DEAD))
    {
     break;
    }
    bool saved = party[who]->savingThrow(BTSAVE_DIFFICULTY);
    if (saved)
    {
     snprintf(tmp, 100, "%s saves!", party[who]->name);
     d.drawText(tmp);
    }
    // Still need to handle special damage like poison
    if (party[who]->takeHP((saved ? number[0] >> 1 : number[0])))
    {
     snprintf(tmp, 100, "%s dies!", party[who]->name);
     d.drawText(tmp);
    }
   }
   if (party.checkDead())
    throw BTPartyDead();
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_BACKONE:
   throw BTSpecialBack();
   break;
  case BTSPECIALCOMMAND_FORWARDONE:
   throw BTSpecialForward();
   break;
  case BTSPECIALCOMMAND_TELEPORT:
   throw BTSpecialTeleport(text, number[0], 21 - number[1], number[2], false);
   break;
  case BTSPECIALCOMMAND_GUILD:
  {
   BTScreenSet b("data/guild.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_REVIEW:
  {
   BTScreenSet b("data/review.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_SHOP:
  {
   BTScreenSet b("data/shop.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_DRAWPICTURE:
   d.drawImage(number[0]);
   break;
  case BTSPECIALCOMMAND_CLEARSPECIALAT:
   game->getMap()->setSpecial(number[0], 21 - number[1], BTSPECIAL_NONE);
   break;
  case BTSPECIALCOMMAND_SETSPECIALAT:
   game->getMap()->setSpecial(number[1], 21 - number[2], number[0]);
   break;
  case BTSPECIALCOMMAND_PRINTLABEL:
   d.drawLabel(text);
   break;
  case BTSPECIALCOMMAND_MONSTERJOIN:
  {
   BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
   char tmp[100];
   snprintf(tmp, 100, "%s asks to join the party! Do you say", monsterList[number[0]].getName());
   d.drawText(tmp);
   d.drawText("Yes, or");
   d.drawText("No");
   char key;
   while (true)
   {
    key = IKeybufferGet();
    if ((27 == key) || ('Y' == key) || ('y' == key) || ('N' == key) || ('n' == key))
     break;
   }
   if (('Y' == key) || ('y' == key))
   {
    XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
    if (party.size() >= BT_PARTYSIZE)
    {
     snprintf(tmp, 100, "No room in your party. %s cannot join!", monsterList[number[0]].getName());
     d.drawText(tmp);
    }
    else
    {
     BTPc *pc = new BTPc;
     pc->setName(monsterList[number[0]].getName());
     pc->race = -1;
     pc->job = BTJOB_MONSTER; // Illusions become monster for some reason
     pc->picture = monsterList[number[0]].getPicture();
     pc->monster = number[0];
     pc->ac = monsterList[number[0]].getAc();
     pc->hp = pc->maxHp = monsterList[number[0]].getHp().roll();
     party.push_back(pc);
     d.drawStats();
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_ALLMONSTERSLEAVE:
  {
   XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
   for (int i = party.size(); i > 0; )
   {
    --i;
    if (party[i]->job == BTJOB_MONSTER)
    {
     char tmp[100];
     snprintf(tmp, 100, "%s leaves your party.", party[i]->name);
     d.drawText(tmp);
     party.erase(party.begin() + i);
     d.drawStats();
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_SETCOUNTER:
   game->setCounter(number[0]);
   break;
  case BTSPECIALCOMMAND_ADDCOUNTER:
   game->setCounter(game->getCounter() + number[0]);
   break;
  case BTSPECIALCOMMAND_SETENCOUNTER:
   game->getCombat().addEncounter(number[0]);
   break;
  case BTSPECIALCOMMAND_SETNUMENCOUNTER:
   game->getCombat().addEncounter(number[1], number[0]);
   break;
  case BTSPECIALCOMMAND_BEGINCOMBAT:
   game->getCombat().run(d);
   break;
  case BTSPECIALCOMMAND_GIVEGOLD:
  {
   // Loses extra gold like BTCS.  Should fix.
   XMLVector<BTPc*> &party = game->getParty();
   int gold = number[0] / party.size();
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->giveGold(gold);
   }
   break;
  }
  case BTSPECIALCOMMAND_GIVEXP:
  {
   XMLVector<BTPc*> &party = game->getParty();
   int xp = number[0] / party.size();
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->giveXP(xp);
   }
   break;
  }
  case BTSPECIALCOMMAND_PRESSANYKEY:
   d.drawText("(Press any key)");
   IKeybufferGet();
   break;
  case BTSPECIALCOMMAND_MONSTERLEAVE:
  {
   XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if ((party[i]->job == BTJOB_MONSTER) && (party[i]->monster == number[0]) && (0 == strcmp(party[i]->name, BTGame::getGame()->getMonsterList()[number[0]].getName())))
    {
     char tmp[100];
     snprintf(tmp, 100, "%s leaves your party.", party[i]->name);
     d.drawText(tmp);
     party.erase(party.begin() + i);
     d.drawStats();
     break;
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_SETDIRECTION:
   game->setFacing(number[0]);
   break;
  case BTSPECIALCOMMAND_CLEARSPECIAL:
   game->getMap()->setSpecial(game->getX(), game->getY(), BTSPECIAL_NONE);
   break;
  case BTSPECIALCOMMAND_SETSPECIAL:
   game->getMap()->setSpecial(game->getX(), game->getY(), number[0]);
   break;
  case BTSPECIALCOMMAND_CLEARTEXT:
   d.clearText();
   break;
  case BTSPECIALCOMMAND_TAKEGOLD:
  {
   XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
   int left = number[0];
   for (int i = 0; i < party.size(); ++i)
   {
    left -= party[i]->takeGold(left);
    if (left <= 0)
     break;
   }
   break;
  }
  case BTSPECIALCOMMAND_DRAWFULLPICTURE:
   d.drawFullScreen(text, 0);
   d.refresh();
   break;
  case BTSPECIALCOMMAND_GOTO:
   throw BTSpecialGoto(number[0] - 1);
   break;
  case BTSPECIALCOMMAND_SUBTRACTCOUNTER:
   game->setCounter(game->getCounter() - number[0]);
   break;
  default:
   break;
 }
}

BTSpecialCommand BTSpecialCommand::Guild(BTSPECIALCOMMAND_GUILD);

BTSpecialConditional::BTSpecialConditional()
{
 type = -1;
}

IShort BTSpecialConditional::getType() const
{
 return type;
}

IBool BTSpecialConditional::isNothing() const
{
 return ((-1 == type) && (0 == thenClause.getType()));
}

void BTSpecialConditional::print(FILE *f) const
{
 char *dollarSign;
 long len;

 if (-1 == type)
 {
  fprintf(f, "DO   ");
  thenClause.print(f);
 }
 else
 {
  fprintf(f, "IF   ");
  dollarSign = strchr(conditionalCommands[type], '$');
  if (dollarSign)
  {
   len = (long)dollarSign - (long)conditionalCommands[type];
   fwrite(conditionalCommands[type], 1, len, f);
   switch (dollarSign[1])
   {
    case 'I':
     fprintf(f, "%s", BTGame::getGame()->getItemList()[number].getName());
     break;
    case 'A':
     fprintf(f, "%s", BTGame::getGame()->getMonsterList()[number].getName());
     break;
    case 'C':
     fprintf(f, "%s", BTGame::getGame()->getJobList()[number]->name);
     break;
    case 'R':
     fprintf(f, "%s", BTGame::getGame()->getRaceList()[number]->name);
     break;
    case 'D':
     fprintf(f, "%s", directions[number]);
     break;
    case '#':
    case 'G':
    case 'F':
     fprintf(f, "%d", number);
     break;
    case '$':
    default:
     fprintf(f, "%s", text);
     break;
   }
   dollarSign += 2;
  }
  else
  {
   dollarSign = conditionalCommands[type];
  }
  fprintf(f, "%s\n", dollarSign);
  fprintf(f, "    THEN ");
  thenClause.print(f);
  fprintf(f, "    ELSE ");
  elseClause.print(f);
 }
}

void BTSpecialConditional::read(BinaryReadFile &f)
{
 f.readShort(type);
 f.readUByteArray(26, (IUByte *)text);
 f.readShort(number);
 thenClause.read(f);
 elseClause.read(f);
}

void BTSpecialConditional::run(BTDisplay &d) const
{
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
 bool truth = true;
 switch (type)
 {
  case BTCONDITION_ANYONEITEM:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->hasItem(number))
    {
     truth = true;
     break;
    }
   }
   break;
  }
  case BTCONDITION_EVERYONEITEM:
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (!party[i]->hasItem(number))
    {
     truth = false;
     break;
    }
   }
   break;
  }
  case BTCONDITION_LASTINPUT:
   truth = (strcmp(BTGame::getGame()->getLastInput().c_str(), text) == 0);
   break;
  case BTCONDITION_ANSWERYES:
  {
   d.drawText(text);
   std::string ans = d.readString("", 3);
   truth = ((strcmp(ans.c_str(), "y") == 0) ||
    (strcmp(ans.c_str(), "yes") == 0));
   break;
  }
  case BTCONDITION_GROUPFACING:
   truth = (BTGame::getGame()->getFacing() == number);
   break;
  case BTCONDITION_MONSTERINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if ((party[i]->job == BTJOB_MONSTER) && (party[i]->monster == number) && (0 == strcmp(party[i]->name, BTGame::getGame()->getMonsterList()[number].getName())))
     truth = true;
   break;
  }
  case BTCONDITION_JOBINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i]->race == number)
     truth = true;
   break;
  }
  case BTCONDITION_COUNTERGREATER:
   truth = ((BTGame::getGame()->getCounter() > number) ? true : false);
   break;
  case BTCONDITION_COUNTEREQUAL:
   truth = ((BTGame::getGame()->getCounter() == number) ? true : false);
   break;
  case BTCONDITION_RANDOM:
   truth = (BTDice(1, 100).roll() <= number);
   break;
  case BTCONDITION_RACEINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i]->race == number)
     truth = true;
   break;
  }
  case BTCONDITION_CANTAKEGOLD:
  {
   int gold = 0;
   truth = false;
   for (int i = 0; i < party.size(); ++i)
   {
    gold += party[i]->getGold();
    if (gold > number)
     truth = true;
   }
   break;
  }
  default:
   break;
 }
 if (truth)
  thenClause.run(d);
 else
  elseClause.run(d);
}

void BTSpecialConditional::setType(IShort val)
{
 type = val;
}

BTSpecial::BTSpecial()
{
 name[0] = 0;
}

BTSpecial::BTSpecial(BinaryReadFile &f)
{
 IUShort conditionType;
 IUByte unknown[96];

 f.readUByteArray(25, (IUByte *)name);
 f.readUByte(unknown[0]);
 for (int i = 0; i < 20; i++)
 {
  operation[i].read(f);
  if (-99 == operation[i].getType())
  {
   operation[i].setType(-1);
   break;
  }
 }
}

const char *BTSpecial::getName() const
{
 return name;
}

void BTSpecial::print(FILE *f) const
{
 int i, last;

 fprintf(f, "%s\n", name);
 for (last = 20; last > 1; last--)
 {
  if (!operation[last - 1].isNothing())
  {
   break;
  }
 }
 for (i = 0; i < last; i++)
 {
  fprintf(f, "%2d. ",i + 1);
  operation[i].print(f);
 }
}

void BTSpecial::run(BTDisplay &d) const
{
 try
 {
  int i = 0;
  while (i < 20)
  {
   try
   {
    operation[i].run(d);
    ++i;
   }
   catch (const BTSpecialGoto &g)
   {
    i = g.line;
   }
  }
 }
 catch (const BTSpecialStop &)
 {
 }
}

BTMap::BTMap(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByteArray(25, (IUByte *)name);
 f.readUByte(unknown);
 f.readShort(type);
 f.readShort(level);
 f.readShort(monsterLevel);
 f.readShort(monsterChance);
 f.readUByteArray(9, (IUByte *)filename);
 f.readUByte(unknown);
 for (int y = 0; y < 22; y++)
 {
  for (int x = 0; x < 22; x++)
  {
   square[y][x].read(f);
  }
 }
 int i;
 try {
  for (i = 0; i < 30; i++)
  {
   specials[i] = new BTSpecial(f);
  }
 }
 catch (FileException e)
 {
  for (; i < 30; i++)
  {
   specials[i] = NULL;
  }
 }
}

BTMap::~BTMap()
{
 for (int i = 0; i < 30; i++)
 {
  if (specials[i])
  {
   delete specials[i];
  }
 }
}

void BTMap::setSpecial(IShort x, IShort y, IShort special)
{
 square[y][x].setSpecial(special);
}

const char *BTMap::getFilename() const
{
 return filename;
}

IShort BTMap::getLevel() const
{
 return level;
}

IShort BTMap::getMonsterChance() const
{
 return monsterChance;
}

IShort BTMap::getMonsterLevel() const
{
 return monsterLevel;
}

const char *BTMap::getName() const
{
 return name;
}

const BTSpecial *BTMap::getSpecial(IShort num) const
{
 return specials[num];
}

const BTMapSquare &BTMap::getSquare(IShort y, IShort x) const
{
 return square[y][x];
}

IShort BTMap::getType() const
{
 return type;
}
