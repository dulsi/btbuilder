#ifndef __SPELL_H
#define __SPELL_H
/*-------------------------------------------------------------------------*\
  <spell.h> -- Spell header file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <istdlib.h>
#include <file.h>
#include "dice.h"

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

class BTSpell
{
 public:
  BTSpell(BinaryReadFile &f);
  BTSpell();

  const char *getName() const;
  IShort getArea() const;
  const char *getCode() const;
  const BTDice &getDice() const;
  IShort getDuration() const;
  const char *getEffect() const;
  IShort getEffectiveRange() const;
  IShort getExtra() const;
  IShort getRange() const;
  IShort getSp() const;
  IShort getType() const;

 private:
  char name[29];
  char code[5];
  IShort caster;
  IShort level;
  IShort sp;
  IShort range;
  IShort effectiveRange;
  IShort type;
  IShort area;
  BTDice dice;
  IShort duration;
  IShort extra;
  char effect[22];
};

#endif

