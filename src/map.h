#ifndef __MAP_H
#define __MAP_H
/*-------------------------------------------------------------------------*\
  <map.h> -- Map header file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "map.h"

/*
[map]
name: char[25]
unknown: byte
type: short {dungeon 1, dungeon 2, city, wilderness}
level: short {0 to 20}
monster chance: short {0 to 100}
monster level: short {0 to 20}
file name: char[9]
unknown: byte
square: mapsquare[22][22]
specials: special[]

[mapsquare]
wallinfo: walldata
unknown: byte
special: short

[walldata]
upperwall: bits[2] {blank, wall, door, secret door}
rightwall: bits[2]
lowerwall: bits[2]
leftwall: bits[2]

[special]
name: char[25]
operation: condition[] (20 conditions or until condition type 0xFF9D)

[condition]
type: short
unknown: byte[28]
then: command
else: command

[command]
type: short
unknown: byte[32]
*/

class BTMapSquare
{
 public:
  BTMapSquare();
  void read(BinaryReadFile &f);

 private:
  IUByte wallInfo;
  IShort special;
};

class BTSpecial
{
 public:
  BTSpecial();
  BTSpecial(BinaryReadFile &f);

  char *getName();

 private:
  char name[25];
};

class BTMap
{
 public:
  BTMap(BinaryReadFile &f);

 private:
  char name[25];
  IShort type;
  IShort level;
  IShort monsterChance;
  IShort monsterLevel;
  char filename[9];
  BTMapSquare square[22][22];
};

#endif

