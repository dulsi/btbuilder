#ifndef __MAP_H
#define __MAP_H
/*-------------------------------------------------------------------------*\
  <map.h> -- Map header file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>

/*
[map]
name: char[25]
unknown: byte
type: short {dungeon 1, dungeon 2, city, wilderness}
level: short {0 to 20}
monster level: short {0 to 20}
monster chance: short {0 to 100}
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
text: char[26] (text, monster name)
number: short (item, local flag, direction, monster, class, global flag,
  number, race)
then: command
else: command

[command]
type: short
text: char[26] (text)
number: short[3]
*/

class BTMapSquare
{
 public:
  BTMapSquare();

  IShort getWall(IShort dir) const;
  IShort getSpecial() const;
  void read(BinaryReadFile &f);

 private:
  IUByte wallInfo;
  IShort special;
};

class BTSpecialCommand
{
 public:
  BTSpecialCommand();

  IShort getType() const;
  void print(FILE *f) const;
  void read(BinaryReadFile &f);

 private:
  IShort type;
  char text[26];
  IUShort number[3];
};

class BTSpecialConditional
{
 public:
  BTSpecialConditional();

  IShort getType() const;
  IBool isNothing() const;
  void print(FILE *f) const;
  void read(BinaryReadFile &f);
  void setType(IShort val);

 private:
  IShort type;
  char text[26];
  IShort number;
  BTSpecialCommand thenClause;
  BTSpecialCommand elseClause;
};

class BTSpecial
{
 public:
  BTSpecial();
  BTSpecial(BinaryReadFile &f);

  const char *getName() const;
  void print(FILE *f) const;

 private:
  char name[25];
  BTSpecialConditional operation[20];
};

class BTMap
{
 public:
  BTMap(BinaryReadFile &f);
  ~BTMap();

  const char *getFilename() const;
  IShort getLevel() const;
  IShort getMonsterChance() const;
  IShort getMonsterLevel() const;
  const char *getName() const;
  const BTSpecial *getSpecial(IShort num) const;
  const BTMapSquare &getSquare(IShort y, IShort x) const;
  IShort getType() const;

 private:
  char name[25];
  IShort type;
  IShort level;
  IShort monsterChance;
  IShort monsterLevel;
  char filename[9];
  BTMapSquare square[22][22];
  BTSpecial *specials[30];
};

#endif
