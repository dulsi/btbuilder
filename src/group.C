/*-------------------------------------------------------------------------*\
  <group.C> -- Group implementation file

  Date      Programmer  Description
  10/25/09  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "group.h"

void BTGroup::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("member", &member);
}

