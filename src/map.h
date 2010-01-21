#ifndef __MAP_H
#define __MAP_H
/*-------------------------------------------------------------------------*\
  <map.h> -- Map header file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "display.h"

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

class BTMapSquare : public XMLObject
{
 public:
  BTMapSquare();

  IShort getWall(IShort dir) const;
  IShort getSpecial() const;
  void read(BinaryReadFile &f);
  void setSpecial(IShort s);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTMapSquare; }

 private:
  int wallInfo[4];
  IShort special;
};

class BTSpecialError
{
 public:
  BTSpecialError(const std::string &e) : error(e) {}

  std::string error;
};

class BTSpecialBack
{
 public:
  BTSpecialBack() {}
};

class BTSpecialDead
{
 public:
  BTSpecialDead() {}
};

class BTSpecialFlipGoForward
{
 public:
  BTSpecialFlipGoForward() {}
};

class BTSpecialForward
{
 public:
  BTSpecialForward() {}
};

class BTSpecialTeleport
{
 public:
  BTSpecialTeleport(const char *m, int x1, int y1, int f, bool a) : map(m), x(x1), y(y1), facing(f), activate(a) {}

  std::string map;
  int x, y, facing;
  bool activate;
};

class BTSpecialGoto
{
 public:
  BTSpecialGoto(int l) : line(l) {}

  int line;
};

class BTSpecialQuit
{
 public:
  BTSpecialQuit() {}
};

class BTSpecialStop
{
 public:
  BTSpecialStop() {}
};

class BTSpecialCommand : public XMLObject
{
 public:
  BTSpecialCommand();
  BTSpecialCommand(IShort t) : type(t) { text = new char[1]; text[0] = 0; }
  ~BTSpecialCommand();

  IShort getType() const;
  void print(FILE *f) const;
  void read(BinaryReadFile &f);
  void run(BTDisplay &d) const;
  virtual void serialize(ObjectSerializer* s);

  static BTSpecialCommand Guild;

 private:
  IShort type;
  char *text;
  IUShort number[3];
};

class BTSpecialConditional : public XMLObject
{
 public:
  BTSpecialConditional();
  ~BTSpecialConditional();

  IShort getType() const;
  IBool isNothing() const;
  void print(FILE *f) const;
  void read(BinaryReadFile &f);
  void run(BTDisplay &d) const;
  void setType(IShort val);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecialConditional; }

 private:
  IShort type;
  char *text;
  IShort number;
  BTSpecialCommand thenClause;
  BTSpecialCommand elseClause;
};

class BTSpecial : public XMLObject
{
 public:
  BTSpecial();
  BTSpecial(BinaryReadFile &f);
  ~BTSpecial();

  const char *getName() const;
  void print(FILE *f) const;
  void run(BTDisplay &d) const;
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecial; }

 private:
  char *name;
  XMLVector<BTSpecialConditional*> operation;
};

class BTMap : public XMLObject
{
 public:
  BTMap(BinaryReadFile &f);
  BTMap();
  ~BTMap();

  void setSpecial(IShort x, IShort y, IShort special);
  const char *getFilename() const;
  IShort getLevel() const;
  int getLight() const;
  IShort getMonsterChance() const;
  IShort getMonsterLevel() const;
  const char *getName() const;
  const BTSpecial *getSpecial(IShort num) const;
  const BTMapSquare &getSquare(IShort y, IShort x) const;
  IShort getType() const;
  IShort getXSize() const;
  IShort getYSize() const;
  void setFilename(const char *f);
  virtual void serialize(ObjectSerializer* s);

 private:
  char *name;
  IShort type;
  IShort level;
  IShort xSize;
  IShort ySize;
  IShort monsterChance;
  IShort monsterLevel;
  char *filename;
  XMLVector<BTMapSquare*> square;
  XMLVector<BTSpecial*> specials;
};

#endif

