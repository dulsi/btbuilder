/*-------------------------------------------------------------------------*\
  <map.C> -- Map implementation file

  Date      Programmer  Description
  11/12/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "map.h"
#include "game.h"

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
    fprintf(f, "%s", classTypes[number[count++]]);
    break;
   case 'R':
    fprintf(f, "%s", raceTypes[number[count++]]);
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
     fprintf(f, "%s", classTypes[number]);
     break;
    case 'R':
     fprintf(f, "%s", raceTypes[number]);
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

void BTSpecialConditional::setType(IShort val)
{
 type = val;
}

BTSpecial::BTSpecial()
{
 name[0] = 0;
}

const char *BTSpecial::getName() const
{
 return name;
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

