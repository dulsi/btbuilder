#ifndef __GROUP_H
#define __GROUP_H
/*-------------------------------------------------------------------------*\
  <group.h> -- Group header file

  Date      Programmer  Description
  10/25/09  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"

class BTGroup : public XMLObject
{
 public:
  BTGroup() {}
  ~BTGroup() {}

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTGroup; }

  std::string name;
  std::vector<std::string> member;
};

#endif
