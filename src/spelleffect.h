#ifndef __SPELLEFFECT_H
#define __SPELLEFFECT_H
/*-------------------------------------------------------------------------*\
  <spelleffect.h> -- Spell Effect header file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "istdlib.h"
#include <file.h>
#include "xmlserializer.h"

class BTSpellEffect : public XMLObject
{
 public:
  BTSpellEffect(int s, int x, int cl, int d, int g, int t, BitField &r);

  virtual void serialize(ObjectSerializer *s);

  int spell;
  int expiration;
  bool first;
  int casterLevel;
  int distance;
  int group;
  int target;
  BitField resists;
};

#endif

