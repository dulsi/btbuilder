#ifndef __KEYCOMMAND_H
#define __KEYCOMMAND_H
/*-------------------------------------------------------------------------*\
  <keycommand.h> -- Key command header file
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "map.h"

class BTKeyCommand : public XMLObject
{
 public:
  void run(BTDisplay &d, BTSpecialContext *context);

  void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTKeyCommand; }
  static void readXML(const char *filename, XMLVector<BTKeyCommand*> &keyList);

  unsigned int key;
  BTSpecialBody body;
};

#endif
