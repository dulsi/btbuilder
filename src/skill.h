#ifndef __SKILL_H
#define __SKILL_H
/*-------------------------------------------------------------------------*\
  <skill.h> -- Skill header file

  Date      Programmer  Description
  10/11/09  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "dice.h"
#include "xmlserializer.h"

#define BTSKILLUSE_AUTOCOMBAT 0
#define BTSKILLUSE_MAGIC      3

#define BTSKILLSPECIAL_DISARM 0
#define BTSKILLSPECIAL_SONG   2

class BTSkill : public XMLObject
{
 public:
  BTSkill() : use(-1), effect(-1), special(-1), limited(false)
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTSkill() { delete [] name; }

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSkill; }
  static void readXML(const char *filename, XMLVector<BTSkill*> &skill);

  char *name;
  int use;
  int effect;
  bool limited;
  int special;
};

class BTSkillList : public ValueLookup, public XMLVector<BTSkill*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
};

#endif
