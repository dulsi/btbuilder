/*-------------------------------------------------------------------------*\
  <spelleffect.C> -- Spell Effect implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spelleffect.h"

BTSpellEffect::BTSpellEffect(int s, int x, int cl, int d, int g, int t, BitField &r)
 : spell(s), expiration(x), first(true), casterLevel(cl), distance(d), group(g), target(t), resists(r)
{
}

void BTSpellEffect::serialize(ObjectSerializer *s)
{
 s->add("spell", &spell);
 s->add("expiration", &expiration);
}
