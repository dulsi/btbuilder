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
 switch (type)
 {
  case BTSPECIALCOMMAND_STOP:
   throw BTSpecialStop();
   break;
  case BTSPECIALCOMMAND_PRINT:
   d.drawText(text);
   break;
  case BTSPECIALCOMMAND_FORWARDONE:
   throw BTSpecialForward();
   break;
  case BTSPECIALCOMMAND_GUILD:
   adventurerGuild(d);
   break;
  case BTSPECIALCOMMAND_PRINTLABEL:
   d.drawLabel(text);
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

void BTSpecialCommand::adventurerGuild(BTDisplay &d) const
{
 int state = GUILDSTATE_MAIN;
 unsigned char key = ' ';
 BTPc *pc = NULL;

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
      case 'C':
      case 'c':
       state = GUILDSTATE_SELECTRACE;
       break;
      case 'E':
      case 'e':
       throw BTSpecialFlipGoForward();
       break;
      default:
       break;
      case 'D':
      case 'd':
       state = GUILDSTATE_DISKOPS;
       break;
     };
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
    for (int i = 0; i < 8 /*job.size()*/; ++i)
    {
     char line[50];
     snprintf(line, 50, "%d) %s", i + 1, job[i]->name);
     d.drawText(line);
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
      int c = key - '1';
      if (c < job.size())
      {
       pc->job = c;
       state = GUILDSTATE_SELECTNAME;
      }
     }
    }
    break;
   }
   case GUILDSTATE_SELECTNAME:
   {
//    int i;
    d.clearText();
    d.drawText("Enter the new new member's name.");
/*    XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
    for (i = 0; i < race.size(); ++i)
    {
     char line[50];
     snprintf(line, 50, "%d) %s", i + 1, race[i]->name);
     d.drawText(line);
    }*/
    while (state == GUILDSTATE_SELECTNAME)
    {
     key = IKeybufferGet();
     if (27 == key)
      state = GUILDSTATE_MAIN;
/*     else if (('1' <= key) && ('9' >= key))
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
     }*/
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
 if (-1 == type)
  thenClause.run(d);
 else
 {
 }
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
