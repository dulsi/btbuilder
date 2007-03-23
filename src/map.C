/*-------------------------------------------------------------------------*\
  <map.C> -- Map implementation file

  Date      Programmer  Description
  11/12/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
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
  case BTSPECIALCOMMAND_GETINPUT:
   game->setLastInput(d.readString("", 13));
   break;
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
   adventurerGuild(d);
   break;
  case BTSPECIALCOMMAND_SHOP:
   shop(d);
   break;
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
  case BTSPECIALCOMMAND_GIVEXP:
  {
   XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
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
  default:
   break;
 }
}

#define GUILDSTATE_MAIN 0
#define GUILDSTATE_QUIT 1
#define GUILDSTATE_SELECTRACE 2
#define GUILDSTATE_DISKOPS    3
#define GUILDSTATE_SELECTJOB  4
#define GUILDSTATE_SELECTNAME 5
#define GUILDSTATE_REMOVE 6
#define GUILDSTATE_ADD 7
#define GUILDSTATE_SAVEPARTY 8
#define GUILDSTATE_DELETE 9
#define GUILDSTATE_RENAME 10
#define GUILDSTATE_NOPARTY 11

void BTSpecialCommand::adventurerGuild(BTDisplay &d) const
{
 int state = GUILDSTATE_MAIN;
 unsigned char key = ' ';
 BTPc *pc = NULL;
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();

 d.drawImage(40);
 d.drawLabel("The Guild");
 while (true)
 {
  switch (state)
  {
   case GUILDSTATE_MAIN:
    if (NULL != pc)
    {
     delete pc;
     pc = NULL;
    }
    d.clearText();
    d.drawText("Thou art in the Guild of Adventurers.");
    d.drawText("Add member");
    d.drawText("Remove member");
    d.drawText("Create a member");
    d.drawText("Save party");
    d.drawText("Quit game");
    d.drawText("Exit the guild");
    d.drawText("Disk options");
    while (state == GUILDSTATE_MAIN)
    {
     key = IKeybufferGet();
     switch (key)
     {
      case 'Q':
      case 'q':
       state = GUILDSTATE_QUIT;
       break;
      case 'A':
      case 'a':
       state = GUILDSTATE_ADD;
       break;
      case 'R':
      case 'r':
       state = GUILDSTATE_REMOVE;
       break;
      case 'C':
      case 'c':
       state = GUILDSTATE_SELECTRACE;
       break;
      case 'E':
      case 'e':
      {
       bool live = false;
       for (int i = 0; i < party.size(); ++i)
       {
        if (party[i]->isAlive())
        {
         live = true;
         break;
        }
       }
       if (!live)
        state = GUILDSTATE_NOPARTY;
       else
        throw BTSpecialFlipGoForward();
       break;
      }
      case 'D':
      case 'd':
       state = GUILDSTATE_DISKOPS;
       break;
      default:
       break;
     };
    }
    break;
   case GUILDSTATE_NOPARTY:
    d.clearText();
    d.drawText("You must have a live party to enter the city.");
    key = IKeybufferGet();
    state = GUILDSTATE_MAIN;
    break;
   case GUILDSTATE_ADD:
   {
    d.clearText();
    if (0 == roster.size())
    {
     d.drawText("No adventurers available.");
     key = IKeybufferGet();
    }
    else if (party.size() >= BT_PARTYSIZE)
    {
     d.drawText("Party is full.");
     key = IKeybufferGet();
    }
    else
    {
     BTDisplay::selectItem *list = new BTDisplay::selectItem[roster.size()];
     for (int i = 0; i < roster.size(); ++i)
     {
      list[i].name = roster[i]->name;
     }
     int start(0), select(0), found;
     while (d.selectList(list, roster.size(), start, select))
     {
      for (found = 0; found < party.size(); ++found)
       if (0 == strcmp(roster[select]->name, party[found]->name))
       {
        char tmp[50];
        d.clearText();
        snprintf(tmp, 50, "%s is already in the party.", roster[select]->name);
        d.drawText(tmp);
        key = IKeybufferGet();
        break;
       }
      if (found >= party.size())
      {
       party.push_back(roster[select]);
       d.drawStats();
       if (party.size() >= BT_PARTYSIZE)
        break;
      }
     }
     delete [] list;
    }
    state = GUILDSTATE_MAIN;
    break;
   }
   case GUILDSTATE_REMOVE:
    d.clearText();
    if (party.size())
    {
     d.drawText("Pick a party member to remove.");
    }
    else
    {
     d.drawText("The party is empty!");
    }
    while (state == GUILDSTATE_REMOVE)
    {
     key = IKeybufferGet();
     if ((27 == key) || (0 == party.size()))
      state = GUILDSTATE_MAIN;
     else if (('1' <= key) && ('9' >= key))
     {
      int n =  key - '1';
      if (n < party.size())
      {
       // Add monster to save file
       party.erase(party.begin() + n);
       d.drawStats();
       state = GUILDSTATE_MAIN;
      }
     }
    }
    break;
   case GUILDSTATE_QUIT:
    d.clearText();
    d.drawText("");
    d.drawText("Quit the game?");
    d.drawText("Yes, or");
    d.drawText("No");
    while (state == GUILDSTATE_QUIT)
    {
     key = IKeybufferGet();
     if (('N' == key) || ('n' == key) || (27 == key))
      state = GUILDSTATE_MAIN;
     else if (('Y' == key) || ('y' == key))
      throw BTSpecialQuit();
    }
    break;
   case GUILDSTATE_SELECTRACE:
   {
    int i;
    d.clearText();
    d.drawText("Select a race for your new character:");
    XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
    for (i = 0; i < race.size(); ++i)
    {
     char line[50];
     snprintf(line, 50, "%d) %s", i + 1, race[i]->name);
     d.drawText(line);
    }
    while (state == GUILDSTATE_SELECTRACE)
    {
     key = IKeybufferGet();
     if (27 == key)
      state = GUILDSTATE_MAIN;
     else if (('1' <= key) && ('9' >= key))
     {
      int r =  key - '1';
      if (r < race.size())
      {
       if (NULL == pc)
        pc = new BTPc;
       pc->race = r;
       for (i = 0; i < BT_STATS; ++i)
        pc->stat[i] = race[r]->stat[i].roll();
       state = GUILDSTATE_SELECTJOB;
      }
     }
    }
    break;
   }
   case GUILDSTATE_SELECTJOB:
   {
    char stat[2][10];
    int i;
    d.clearText();
    for (i = 0; i < BT_STATS; i++)
    {
     snprintf(stat[i % 2], 10, "%s: %d", statAbbrev[i], pc->stat[i]);
     if (i % 2 == 1)
      d.draw2Column(stat[0], stat[1]);
    }
    if (i % 2 == 1)
     d.draw2Column(stat[0], "");
    XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
    int count = 1;
    for (i = 0; i < job.size(); ++i)
    {
     if (!job[i]->advanced)
     {
      char line[50];
      snprintf(line, 50, "%d) %s", count++, job[i]->name);
      d.drawText(line);
     }
    }
    d.drawText("(Reroll)");
    while (state == GUILDSTATE_SELECTJOB)
    {
     key = IKeybufferGet();
     if (27 == key)
      state = GUILDSTATE_MAIN;
     else if (('R' == key) || ('r' == key))
      state = GUILDSTATE_SELECTRACE;
     else if (('1' <= key) && ('9' >= key))
     {
      count = key - '1';
      for (i = 0; i < job.size(); i++)
      {
       if (!job[i]->advanced)
       {
        if (count == 0)
        {
         pc->job = i;
         pc->hp = pc->maxHp = BTDice(1, 14, 14).roll() + ((pc->stat[BTSTAT_CN] > 14) ? pc->stat[BTSTAT_CN] - 14 : 0);
         pc->gold = BTDice(1, 61, 110).roll();
         state = GUILDSTATE_SELECTNAME;
        }
        --count;
       }
      }
     }
    }
    break;
   }
   case GUILDSTATE_SELECTNAME:
   {
    d.clearText();
    d.drawText("Enter the new new member's name.");
    std::string nm = d.readString(">", 13);
    int found;
    for (found = 0; found < roster.size(); ++found)
    {
     if (0 == strcmp(roster[found]->name, nm.c_str()))
     {
      d.clearText();
      d.drawText("There is already a person by that name.");
      key = IKeybufferGet();
      break;
     }
    }
    if (found >= roster.size())
    {
     pc->setName(nm.c_str());
     roster.push_back(pc);
     pc = NULL;
     state = GUILDSTATE_MAIN;
    }
    break;
   }
   case GUILDSTATE_DISKOPS:
    d.clearText();
    d.drawText("Delete a member");
    d.drawText("Rename a member");
    d.drawText("Exit disk options");
    while (state == GUILDSTATE_DISKOPS)
    {
     key = IKeybufferGet();
     switch (key)
     {
      case 'E':
      case 'e':
       state = GUILDSTATE_MAIN;
       break;
      default:
       break;
     };
    }
    break;
   default:
    break;
  }
 }
}

void BTSpecialCommand::shop(BTDisplay &d) const
{
 unsigned char key = ' ';
 BTPc *pc = NULL;
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();

 d.drawImage(39);
 d.drawLabel("The Shoppe");
 while (true)
 {
  if (pc == NULL)
  {
   d.clearText();
   d.drawText("Welcome to Garth's Equipment Shoppe, oh wealthy travellers!");
   d.drawText("Which of you is interested in my fine ware?");
   while (pc == NULL)
   {
    key = IKeybufferGet();
    if (27 == key)
     throw BTSpecialFlipGoForward();
    else if (('1' <= key) && ('9' >= key))
    {
     int p = key - '1';
     if (p < party.size())
     {
      pc = party[p];
     }
    }
   }
  }
  else
  {
   char line[100];
   bool refresh(true);
   snprintf(line, 100, "Greetings, %s. Would you like to:", pc->name);
   while (pc != NULL)
   {
    if (refresh)
    {
     d.clearText();
     d.drawText(line);
     d.drawText("");
     d.drawText("Buy an item.");
     d.drawText("Sell an item.");
     d.drawText("Identify item.");
     d.drawText("Done.");
     refresh = false;
    }
    key = IKeybufferGet();
    switch (key)
    {
     case 'D':
     case 'd':
      pc = NULL;
      break;
     case 'B':
     case 'b':
     {
      refresh = true;
      if (pc->isEquipmentFull())
      {
       d.clearText();
       d.drawText("Your pockets are full."); // You have no items
       IKeybufferGet();
      }
      else
      {
       BTDisplay::selectItem *list = new BTDisplay::selectItem[9];
       for (int i = 0; (i < itemList.size()) && (i < 9); ++i)
       {
        if (!itemList[i].canUse(pc))
         list[i].first = '@';
        list[i].name = itemList[i].getName();
        list[i].value = itemList[i].getPrice();
       }
       int start(0), select(0), found;
       while (d.selectList(list, ((9 < itemList.size()) ? 9 : itemList.size()), start, select))
       {
        if (pc->getGold() < itemList[select].getPrice())
        {
         d.clearText();
         d.drawText("Not enough gold."); // You have no items
         IKeybufferGet();
        }
        else
        {
         d.drawLast("Done!");
         IKeybufferGet();
         pc->takeGold(itemList[select].getPrice());
         pc->giveItem(select, true, itemList[select].getTimesUsable());
         if (pc->isEquipmentFull())
          break;
        }
       }
       delete [] list;
      }
      break;
     }
     case 'S':
     case 's':
     {
      refresh = true;
      if (pc->isEquipmentEmpty())
      {
       d.clearText();
       d.drawText("You have no items.");
       IKeybufferGet();
      }
      else
      {
       BTDisplay::selectItem *list = new BTDisplay::selectItem[BT_ITEMS];
       int len = 8;
       for (int i = 0; i < BT_ITEMS; ++i)
       {
        int id = pc->getItem(i);
        if (id == BTITEM_NONE)
        {
         len = i;
         break;
        }
        if (pc->isEquipped(i))
         list[i].first = '*';
        else if (!itemList[id].canUse(pc))
         list[i].first = '@';
        list[i].name = itemList[id].getName();
        list[i].value = itemList[id].getPrice() / 2;
       }
       int start(0), select(0), found;
       while (d.selectList(list, len, start, select))
       {
       }
       delete [] list;
      }
      break;
     }
     default:
      break;
    }
   }
  }
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
  case BTCONDITION_JOBINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i]->race == number)
     truth = true;
   break;
  }
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
