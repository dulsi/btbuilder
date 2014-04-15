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
#include <typeinfo>

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
  void setWall(IShort dir, IShort wall);
  void setSpecial(IShort s);
  virtual void serialize(ObjectSerializer* s);
  void write(BinaryWriteFile &f);

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
  BTSpecialGoto(const std::string &l) : label(l) {}

  std::string label;
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

class BTSpecialOperation : public XMLObject
{
 public:
  virtual BTSpecialOperation *clone() const = 0;
  virtual IBool isNothing() const = 0;
  virtual std::string print() const = 0;
  virtual void print(FILE *f, int indent) const = 0;
  virtual void run(BTDisplay &d) const = 0;
};

class BTSpecialBody : public BTSpecialOperation
{
 public:
  friend class BTSpecialConditional;
  friend class BTSpecial;

 public:
  BTSpecialBody() { }
  BTSpecialBody(const BTSpecialBody &copy);

  void addOperation(BTSpecialOperation *op) { ops.push_back(op); }
  void eraseOperation(BTSpecialOperation *op);
  int findLabel(const std::string &l) const;
  BTSpecialOperation *getOperation(int line);
  void insertOperation(int line, BTSpecialOperation *op) { ops.insert(ops.begin() + line, op); }
  void insertOperation(BTSpecialOperation *before, BTSpecialOperation *op);
  void replaceOperation(BTSpecialOperation *opOld, BTSpecialOperation *opNew);
  BTSpecialOperation *clone() const;
  IBool isNothing() const;
  void moveFrom(BTSpecialBody *body);
  int numOfOperations(bool recursive) const;
  std::string print() const;
  void print(FILE *f, int indent) const;
  void run(BTDisplay &d) const;
  void runFromLine(BTDisplay &d, int line) const;
  void serialize(ObjectSerializer* s);
  void upgradeToLabel(BitField &labelNeeded);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecialBody; }

 protected:
  XMLVector<BTSpecialOperation*> ops;
};

class BTSpecialCommand : public BTSpecialOperation
{
 public:
  BTSpecialCommand();
  BTSpecialCommand(IShort t) : type(t) { text = new char[1]; text[0] = 0; }
  ~BTSpecialCommand();

  BTSpecialOperation *clone() const;
  IShort getType() const;
  std::string getText() const;
  IUShort getNumber(int indx) const;
  IBool isNothing() const;
  std::string print() const;
  void print(FILE *f, int indent) const;
  void read(BinaryReadFile &f);
  void run(BTDisplay &d) const;
  void serialize(ObjectSerializer* s);
  void setText(const std::string &t);
  void setNumber(int indx, IUShort value);
  void write(BinaryWriteFile &f);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecialCommand; }

  static BTSpecialCommand Guild;

 private:
  IShort type;
  char *text;
  IUShort number[3];
};

class BTSpecialConditional : public BTSpecialOperation
{
 public:
  BTSpecialConditional();
  BTSpecialConditional(const BTSpecialConditional &copy);
  BTSpecialConditional(IShort t, const char *txt, IShort num);
  ~BTSpecialConditional();

  void addThenOperation(BTSpecialOperation *op) { thenClause.addOperation(op); }
  void addElseOperation(BTSpecialOperation *op) { elseClause.addOperation(op); }
  BTSpecialOperation *clone() const;
  IShort getType() const;
  BTSpecialBody *getThenClause() { return &thenClause; }
  BTSpecialBody *getElseClause() { return &elseClause; }
  IBool isNothing() const;
  std::string print() const;
  void print(FILE *f, int indent) const;
  void read(BinaryReadFile &f);
  void run(BTDisplay &d) const;
  void setType(IShort val);
  void serialize(ObjectSerializer* s);
  void write(BinaryWriteFile &f);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecialConditional; }

 private:
  IShort type;
  char *text;
  IShort number;
  BTSpecialBody thenClause;
  BTSpecialBody elseClause;
};

class BTSpecial : public XMLObject
{
 public:
  BTSpecial();
  BTSpecial(const BTSpecial &copy);
  BTSpecial(BinaryReadFile &f);
  ~BTSpecial();

  BTSpecialBody *getBody();
  const char *getName() const;
  std::string printFlags() const;
  void print(FILE *f) const;
  void run(BTDisplay &d) const;
  void serialize(ObjectSerializer* s);
  void setName(const std::string &nm);
  void write(BinaryWriteFile &f);
  void upgrade();

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSpecial; }

 private:
  char *name;
  BitField flags;
  BTSpecialBody body;
};

class BTMonsterChance : public XMLObject
{
 public:
  BTMonsterChance(int c = 0, int g = 1);

  std::string createString();
  int getChance() const;
  int getGroups() const;
  void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTMonsterChance; }

 private:
  int chance;
  int groups;
};

class BTMap : public XMLObject
{
 public:
  BTMap(BinaryReadFile &f);
  BTMap(int v = 2);
  ~BTMap();

  void addSpecial(BTSpecial *s);
  void checkRandomEncounter(BTDisplay &d) const;
  const char *getFilename() const;
  IShort getLevel() const;
  int getLight() const;
  int getMonsterChance() const;
  IShort getMonsterLevel() const;
  const char *getName() const;
  int getNumOfSpecials() const;
  void generateRandomEncounter(BTDisplay &d, int groups) const;
  BTSpecial *getSpecial(IShort num);
  BTMapSquare &getSquare(IShort y, IShort x);
  IShort getType() const;
  IShort getXSize() const;
  IShort getYSize() const;
  void resize(IShort newXSize, IShort newYSize);
  void setFilename(const char *f);
  void setSpecial(IShort x, IShort y, IShort special);
  void serialize(ObjectSerializer* s);
  void write(BinaryWriteFile &f);
  void upgrade();

 private:
  char *name;
  int version;
  int type;
  IShort level;
  IShort xSize;
  IShort ySize;
  IShort monsterLevel;
  int light;
  char *filename;
  XMLVector<BTMonsterChance*> monsterChance;
  XMLVector<BTMapSquare*> square;
  XMLVector<BTSpecial*> specials;
};

#endif

