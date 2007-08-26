/*-------------------------------------------------------------------------*\
  <spelleffect.C> -- Spell Effect implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spelleffect.h"

BTSpellEffect::BTSpellEffect(int s, int x, int g, int t)
 : spell(s), expiration(x), first(true), group(g), target(t)
{
}

void BTSpellEffect::serialize(ObjectSerializer *s)
{
 s->add("spell", &spell);
 s->add("expiration", &expiration);
}
