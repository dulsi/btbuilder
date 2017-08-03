#ifndef __SERIALRECT_H
#define __SERIALRECT_H
/*-------------------------------------------------------------------------*\
  <searialrect.h> -- SerialRect header file

  Date      Programmer  Description
  03/31/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <SDL.h>
#include "xmlserializer.h"

class SerialRect : public SDL_Rect, public XMLObject
{
 public:
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new SerialRect; }
};

#endif
