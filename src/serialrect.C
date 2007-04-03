/*-------------------------------------------------------------------------*\
  <serialrect.C> -- Serialized rectangle implementation file

  Date      Programmer  Description
  03/31/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "serialrect.h"

void SerialRect::serialize(ObjectSerializer* s)
{
 s->add("x", &x);
 s->add("y", &y);
 s->add("w", &w);
 s->add("h", &h);
}
