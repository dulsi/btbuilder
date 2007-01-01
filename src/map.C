/*-------------------------------------------------------------------------*\
  <map.C> -- Map implementation file

  Date      Programmer  Description
  11/12/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "map.h"

BTMapSquare::BTMapSquare()
 : wallInfo(0), special(-1)
{
}

void BTMapSquare::read(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByte(wallInfo);
 f.readUByte(unknown);
 f.readShort(special);
}

BTSpecial::BTSpecial()
{
 name[0] = 0;
}

char *BTSpecial::getName()
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
  f.readUShort(conditionType);
  f.readUByteArray(96, unknown);
  if (0xFF9D == conditionType)
  {
   break;
  }
 }
}

BTMap::BTMap(BinaryReadFile &f)
{
 IUByte unknown;

 f.readUByteArray(25, (IUByte *)name);
 f.readUByte(unknown);
 f.readShort(type);
 f.readShort(level);
 f.readShort(monsterChance);
 f.readShort(monsterLevel);
 f.readUByteArray(9, (IUByte *)filename);
 f.readUByte(unknown);
 for (int y = 0; y < 22; y++)
 {
  for (int x = 0; x < 22; x++)
  {
   square[y][x].read(f);
  }
 }
 try {
  while (true)
  {
   BTSpecial sp(f);
  }
 }
 catch (FileException e)
 {
 }
}

