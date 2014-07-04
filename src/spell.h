#ifndef __SPELL_H
#define __SPELL_H
/*-------------------------------------------------------------------------*\
  <spell.h> -- Spell header file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "factory.h"
#include "dice.h"
#include "combat.h"
#include "display.h"
#include "manifest.h"

/*
[spell]
name: char[29]
code: char[5]
caster: short {conjurer, magician, sorcerer, wizard, archmage}
level: short {1 to 8}
spell points: short {0 to 255}
range: short {0 to 9}
effective range: short {normal or double}
type: short
area: short {n/a, single target, group target, all monster groups}
healing/damage dice: dice
unknown: byte
duration: short {one turn, short, medium, long, combat, permanent,
  continuous, indefinite}
extra info: short (monster for summon monster and summon illusion,
  amour class bonus for armor class bonus, hit bonus for hit bonus,
  songs to regenerate for regenerate bard songs)
spell effect: char[22];
*/

class BTSpell1;

class BTSpell : public XMLObject
{
 public:
  BTSpell();
  BTSpell(const BTSpell &copy);
  ~BTSpell();

  std::string describeManifest() const;
  const std::string &getName() const;
  int getArea() const;
  int getCaster() const;
  const char *getCode() const;
  int getDuration() const;
  const char *getEffect() const;
  IShort getEffectiveRange() const;
  IShort getLevel() const;
  IShort getRange() const;
  IShort getSp() const;
  void write(BinaryWriteFile &f);

  int activate(BTDisplay &d, const char *activation, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target);
  int cast(BTDisplay &d, const char *caster, int casterGroup, int casterTarget, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target);

  virtual void serialize(ObjectSerializer* s);
  virtual void upgrade();

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);
  static void readXML(const char *filename, XMLVector<BTSpell*> &spell);
  static void writeXML(const char *filename, XMLVector<BTSpell*> &spell);

 protected:
  std::string name;
  char *code;
  int caster;
  IShort level;
  IShort sp;
  IShort range;
  IShort effectiveRange;
  int area;
  int duration;
  char *effect;
  XMLVector<BTManifest*> manifest;

  static int version;
};

class BTSpell1 : public BTSpell
{
 public:
  BTSpell1(BinaryReadFile &f);
  BTSpell1();
  BTSpell1(const BTSpell1 &copy);
  ~BTSpell1();

  const BTDice &getDice() const;
  int getExtra() const;
  int getType() const;
  void upgrade();

  virtual void serialize(ObjectSerializer* s);

 private:
  int type;
  BTDice dice;
  int extra;
};

class BTSpellListCompare : public BTSortCompare<BTSpell>
{
 public:
  int Compare(const BTSpell &a, const BTSpell &b) const;
};

#endif

