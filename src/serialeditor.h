#ifndef __SERIALEDITOR_H
#define __SERIALEDITOR_H
/*-------------------------------------------------------------------------*\
  <serialeditor.h> -- Serialized editor header file

  Date      Programmer  Description
  10/18/13  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "editor.h"

class BTSerializedEditor
{
 public:
  BTSerializedEditor(int num, const char **d, const char **f);
  virtual ~BTSerializedEditor();

  void edit(BTDisplay &d, ObjectSerializer &serial);

 protected:
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  int setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 protected:
  int entries;
  const char **description;
  const char **field;
};

#define FIELDS_MAP 4

class BTMapPropertiesEditor : public BTSerializedEditor
{
 public:
  BTMapPropertiesEditor();

 private:
  static const char *mapDescription[FIELDS_MAP];
  static const char *mapField[FIELDS_MAP];
};

#define FIELDS_ITEM 14
#define FIELDS_MONSTER 23
#define FIELDS_SPELL 18

class BTItemEditor : public BTSerializedEditor
{
 public:
  BTItemEditor();

 private:
  static const char *itemDescription[FIELDS_ITEM];
  static const char *itemField[FIELDS_ITEM];
};

class BTMonsterEditor : public BTSerializedEditor
{
 public:
  BTMonsterEditor();

 private:
  static const char *monsterDescription[FIELDS_MONSTER];
  static const char *monsterField[FIELDS_MONSTER];
};

class BTSpellEditor : public BTSerializedEditor
{
 public:
  BTSpellEditor();

 protected:
  virtual void initActive(ObjectSerializer &serial, BitField &active);
  virtual bool updateActive(ObjectSerializer &serial, BitField &active, int modField);

 private:
  static const char *spellDescription[FIELDS_SPELL];
  static const char *spellField[FIELDS_SPELL];
};

#endif

