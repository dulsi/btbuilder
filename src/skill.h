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

#define BTSKILLUSE_AUTOCOMBAT       0
#define BTSKILLUSE_AUTOCOMBATMELEE  1
#define BTSKILLUSE_AUTOCOMBATRANGED 2
#define BTSKILLUSE_MAGIC            5

#define BTSKILLSPECIAL_DISARM    0
#define BTSKILLSPECIAL_HIDE      1
#define BTSKILLSPECIAL_SONG      2
#define BTSKILLSPECIAL_BAREHANDS 3
#define BTSKILLSPECIAL_RUN       4

class BTSkill : public XMLObject
{
 public:
  BTSkill() : use(-1), effect(-1), after(-1), special(-1), limited(false), defaultDifficulty(0)
  {
   name = new char[1];
   name[0] = 0;
  }

  ~BTSkill() { delete [] name; }

  BTDice *getRoll(int value);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTSkill; }
  static void readXML(const char *filename, XMLVector<BTSkill*> &skill);

  char *name;
  int use;
  int effect;
  int after;
  bool limited;
  int special;
  XMLVector<BTDice*> roll;
  int defaultDifficulty;
  std::string common;
  std::string success;
  std::string failure;
};

class BTSkillList : public ValueLookup, public XMLVector<BTSkill*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

#endif

