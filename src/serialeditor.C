/*-------------------------------------------------------------------------*\
  <serialeditor.C> -- Serialized editor implementation file

  Date      Programmer  Description
  10/18/13  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "serialeditor.h"
#include <algorithm>
#include <sstream>

BTFactoryEditor::BTFactoryEditor(bool s /*= false*/)
 : start(0), current(-1), sorted(s)
{
}

BTFactoryEditor::~BTFactoryEditor()
{
}

BTSerializedEditor::BTSerializedEditor(int num, const char **d, const char **f, bool s /*= false*/)
 : BTFactoryEditor(s), entries(num), description(d), field(f)
{
}

BTSerializedEditor::~BTSerializedEditor()
{
}

void BTSerializedEditor::edit(BTDisplay &d, ObjectSerializer &serial)
{
 BTDisplayConfig *oldConfig = d.getConfig();
 BTDisplayConfig config;
 XMLSerializer parser;
 config.serialize(&parser);
 parser.parse("data/specialedit.xml", true);
 d.setConfig(&config);
 int start(0);
 int current(0);
 BitField active;
 std::vector<BTDisplay::selectItem> list(entries);
 initActive(serial, active);
 int len = setup(serial, active, list);
 d.addSelection(list.data(), len, start, current);
 int key;
 char extra[2] = {BTKEY_DEL, 0};
 while (27 != (key = d.process(extra)))
 {
  d.clearText();
  XMLAction *curField = NULL;
  if (list[current].value < entries)
   curField = serial.find(field[list[current].value], NULL);
  if (key == BTKEY_DEL)
  {
   if (curField)
   {
    int where = 0;
    if (curField->getType() == XMLTYPE_CREATE)
    {
     XMLArray *obj = reinterpret_cast<XMLArray*>(curField->object);
     int where = 0;
     for (int i = current - 1; (i >= 0) && (list[i].value == list[current].value); --i)
     {
      ++where;
     }
     if (where < obj->size())
     {
      obj->erase(where);
      for (int i = current; i < len - 1; ++i)
      {
       list[i].name = list[i + 1].name;
       list[i].value = list[i + 1].value;
      }
      --len;
     }
    }
   }
   else
   {
    delSpecialField(d, serial, list[current].value);
   }
  }
  else
  {
   if (curField)
   {
    int where = 0;
    if (curField->getType() == XMLTYPE_CREATE)
    {
     XMLArray *obj = reinterpret_cast<XMLArray*>(curField->object);
     int where = 0;
     for (int i = current - 1; (i >= 0) && (list[i].value == list[current].value); --i)
     {
      ++where;
     }
     if (where >= obj->size())
     {
      obj->push_back((*reinterpret_cast<XMLObject::create>(curField->data))(field[list[current].value], NULL));
      list.push_back(BTDisplay::selectItem());
      for (int i = list.size() - 1; i > current; --i)
      {
       list[i].name = list[i - 1].name;
       list[i].value = list[i - 1].value;
      }
      len++;
     }
    }
    editField(d, serial, description[list[current].value], curField, list[current].value, where);
    if (curField->getType() == XMLTYPE_OBJECT)
    {
     XMLObject *obj = reinterpret_cast<XMLObject*>(curField->object);
     BTDice *dice = dynamic_cast<BTDice*>(obj);
     if (dice)
      list[current].name = std::string(description[list[current].value]) + ": " + dice->createString();
    }
    else if (curField->getType() != XMLTYPE_CREATE)
    {
     list[current].name = std::string(description[list[current].value]) + ": " + curField->createString();
    }
    else
    {
     XMLArray *obj = reinterpret_cast<XMLArray*>(curField->object);
     list[current].name = std::string(description[list[current].value]) + ": " + obj->get(where)->createString();
    }
   }
   else
   {
    handleSpecialField(d, serial, list[current].value);
   }
  }
  if (updateActive(serial, active, list[current].value))
   len = setup(serial, active, list);
  d.addSelection(list.data(), len, start, current);
 }
 complete(serial);
 d.clearText();
 d.setConfig(oldConfig);
}

void BTSerializedEditor::complete(ObjectSerializer &serial)
{
}

void BTSerializedEditor::delSpecialField(BTDisplay &d, ObjectSerializer &serial, int val)
{
}

void BTSerializedEditor::editField(BTDisplay &d, ObjectSerializer &serial, const char *text, XMLAction *curField, int modField, int where)
{
 int key;
 switch (curField->getType())
 {
  case XMLTYPE_STDSTRING:
  {
   std::string val = curField->createString();
   d.addReadString(std::string(text) + ": ", 100, val);
   key = d.process();
   if ('\r' == key)
    *(reinterpret_cast<std::string*>(curField->object)) = val;
   break;
  }
  case XMLTYPE_STRING:
  {
   std::string val = curField->createString();
   d.addReadString(std::string(text) + ": ", 100, val);
   key = d.process();
   if ('\r' == key)
   {
    char *str = *(reinterpret_cast<char**>(curField->object));
    if (str)
    {
     delete [] str;
    }
    int len = val.length();
    str = new char[len + 1];
    strncpy(str, val.c_str(), len);
    str[len] = 0;
    *(reinterpret_cast<char**>(curField->object)) = str;
   }
   break;
  }
  case XMLTYPE_BOOL:
  {
   BTDisplay::selectItem vals[2];
   vals[0].name = "false";
   vals[1].name = "true";
   int lookupStart(0);
   int lookupCurrent((*(reinterpret_cast<bool*>(curField->object)) ? 1 : 0));
   d.addSelection(vals, 2, lookupStart, lookupCurrent);
   if (27 != d.process())
   {
    *(reinterpret_cast<bool*>(curField->object)) = lookupCurrent;
   }
   break;
  }
  case XMLTYPE_BITFIELD:
  {
   ValueLookup *lookup = reinterpret_cast<ValueLookup*>(curField->data);
   BitField *bits = reinterpret_cast<BitField*>(curField->object);
   BTDisplay::selectItem lookupItem[lookup->size()];
   for (int i = 0; i < lookup->size(); ++i)
   {
    lookupItem[i].name = lookup->getName(i);
    if (bits->isSet(i))
     lookupItem[i].first = '*';
   }
   int lookupStart(0);
   int lookupCurrent(0);
   d.addSelection(lookupItem, lookup->size(), lookupStart, lookupCurrent);
   int key;
   while (27 != (key = d.process()))
   {
    if (bits->toggle(lookupCurrent))
     lookupItem[lookupCurrent].first = '*';
    else
     lookupItem[lookupCurrent].first = 0;
   }
   break;
  }
  case XMLTYPE_INT:
  {
   if (curField->data)
   {
    ValueLookup *lookup = reinterpret_cast<ValueLookup*>(curField->data);
    bool extra = ((curField->extra == EXTRA_NONE) ? false : true);
    BTDisplay::selectItem lookupItem[lookup->size() + (extra ? 1 : 0)];
    int i = 0;
    if (extra)
    {
     lookupItem[0].name = curField->extraText;
     lookupItem[0].value = -1;
     ++i;
    }
    int endIndex = lookup->getEndIndex();
    int lookupCurrent(0);
    int valIndex((*(reinterpret_cast<int*>(curField->object))) + (extra ? 1 : 0));
    for (int curIndex = lookup->getFirstIndex(); curIndex != endIndex; curIndex = lookup->getNextIndex(curIndex))
    {
     lookupItem[i].name = lookup->getName(curIndex);
     lookupItem[i].value = curIndex;
     if (curIndex == valIndex)
      lookupCurrent = i;
     ++i;
    }
    int lookupStart(0);
    d.addSelection(lookupItem, lookup->size() + (extra ? 1 : 0), lookupStart, lookupCurrent);
    if (27 != d.process())
    {
     *(reinterpret_cast<int*>(curField->object)) = lookupItem[lookupCurrent].value;
    }
   }
   else
   {
    std::string val = curField->createString();
    d.addReadString(std::string(text) + ": ", 100, val);
    key = d.process();
    if ('\r' == key)
     *(reinterpret_cast<int*>(curField->object)) = atol(val.c_str());
   }
   break;
  }
  case XMLTYPE_UINT:
  {
   std::string val = curField->createString();
   d.addReadString(std::string(text) + ": ", 100, val);
   key = d.process();
   if ('\r' == key)
    *(reinterpret_cast<unsigned int*>(curField->object)) = atol(val.c_str());
   break;
  }
  case XMLTYPE_INT16:
  {
   std::string val = curField->createString();
   d.addReadString(std::string(text) + ": ", 100, val);
   key = d.process();
   if ('\r' == key)
    *(reinterpret_cast<int16_t*>(curField->object)) = atol(val.c_str());
   break;
  }
  case XMLTYPE_VECTORUINT:
  {
   std::vector<unsigned int> *vec = reinterpret_cast<std::vector<unsigned int> *>(curField->object);
   std::string val;
   for (int i = 0; i < vec->size(); ++i)
   {
    if (i != 0)
     val += ",";
    char convert[30];
    sprintf(convert, "%u", (*vec)[i]);
    val += convert;
   }
   d.addReadString(std::string(text) + ": ", 100, val);
   key = d.process();
   if ('\r' == key)
   {
    int i = 0;
    const char *start = val.c_str();
    for (const char *comma = strchr(val.c_str(), ','); (start) && (*start); ++i)
    {
     if (i < vec->size())
      (*vec)[i] = atol(start);
     else
      vec->push_back(atol(start));
     start = comma;
     if (start)
     {
      if ((*start) == ',')
       ++start;
      comma = strchr(start, ',');
     }
    }
    if (i < vec->size())
     vec->resize(i);
   }
   break;
  }
  case XMLTYPE_OBJECT:
  {
   XMLObject *obj = reinterpret_cast<XMLObject*>(curField->object);
   BTDice *dice = dynamic_cast<BTDice*>(obj);
   if (dice)
   {
    std::ostringstream stream;
    stream << dice->getNumber();
    std::string val = stream.str();
    d.addReadString(std::string(text) + "- Number of Dice: ", 100, val);
    key = d.process();
    if ('\r' == key)
     dice->setNumber(atol(val.c_str()));
    d.clearText();
    stream.str("");
    stream << dice->getType();
    val = stream.str();
    d.addReadString(std::string(text) + "- Type of Dice: ", 100, val);
    key = d.process();
    if ('\r' == key)
     dice->setType(atol(val.c_str()));
    d.clearText();
    stream.str("");
    stream << dice->getModifier();
    val = stream.str();
    d.addReadString(std::string(text) + "- Modifier to Roll: ", 100, val);
    key = d.process();
    if ('\r' == key)
     dice->setModifier(atol(val.c_str()));
   }
   else
    printf("Unsuppported type: %d\n", curField->getType());
   break;
  }
  case XMLTYPE_CREATE:
  {
   XMLArray *obj = reinterpret_cast<XMLArray*>(curField->object);
   handleObject(d, obj->get(where), modField);
   break;
  }
  case XMLTYPE_PICTURE:
  {
   d.addText(text);
   int val(reinterpret_cast<PictureIndex*>(curField->object)->value);
   d.addSelectImage(val);
   key = d.process();
   d.clearImage();
   if ('\r' == key)
    reinterpret_cast<PictureIndex*>(curField->object)->value = val;
   break;
  }
  default:
   printf("Unsuppported type: %d\n", curField->getType());
   break;
 }
 d.clearText();
}

void BTSerializedEditor::initActive(ObjectSerializer &serial, BitField &active)
{
 for (int i = 0; i < entries; ++i)
  active.set(i);
}

void BTSerializedEditor::handleObject(BTDisplay &d, XMLObject *obj, int modField)
{
}

void BTSerializedEditor::handleSpecialField(BTDisplay &d, ObjectSerializer &serial, int val)
{
}

int BTSerializedEditor::setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items)
{
 int current = 0;
 for (int i = 0; i < entries; ++i)
 {
  if (!active.isSet(i))
   continue;
  XMLAction *curField = serial.find(field[i], NULL);
  if (current >= entries)
   items.push_back(BTDisplay::selectItem());
  if (curField->getType() == XMLTYPE_OBJECT)
  {
   XMLObject *obj = reinterpret_cast<XMLObject*>(curField->object);
   items[current].name = std::string(description[i]) + ": " + obj->createString();
   items[current].value = i;
   ++current;
  }
  else if (curField->getType() == XMLTYPE_CREATE)
  {
   XMLArray *obj = reinterpret_cast<XMLArray*>(curField->object);
   for (int k = 0; k < obj->size(); k++)
   {
    if (current >= items.size())
     items.push_back(BTDisplay::selectItem());
    items[current].name = std::string(description[i]) + ": " + obj->get(k)->createString();
    items[current].value = i;
    ++current;
   }
   if (current >= items.size())
    items.push_back(BTDisplay::selectItem());
   items[current].name = std::string(description[i]) + ": <New>";
   items[current].value = i;
   ++current;
  }
  else
  {
   items[current].name = std::string(description[i]) + ": " + curField->createString();
   items[current].value = i;
   ++current;
  }
 }
 return current;
}

bool BTSerializedEditor::updateActive(ObjectSerializer &serial, BitField &active, int modField)
{
 return false;
}

#define MAPLOC_MONSTERCHANCE 4

BTMapPropertiesEditor::BTMapPropertiesEditor()
 : BTSerializedEditor(FIELDS_MAP, mapDescription, mapField)
{
}

void BTMapPropertiesEditor::handleObject(BTDisplay &d, XMLObject *obj, int modField)
{
 if (modField == MAPLOC_MONSTERCHANCE)
 {
  BTMonsterChanceEditor chanceEditor;
  ObjectSerializer serial;
  obj->serialize(&serial);
  chanceEditor.edit(d, serial);
 }
}

const char *BTMapPropertiesEditor::mapDescription[FIELDS_MAP] = { "Name", "Type", "Light", "Monster Level", "Encounter Chance" };
const char *BTMapPropertiesEditor::mapField[FIELDS_MAP] = { "name", "type", "light", "monsterLevel", "monsterChance" };

BTMonsterChanceEditor::BTMonsterChanceEditor()
 : BTSerializedEditor(FIELDS_MONSTERCHANCE, monsterChanceDescription, monsterChanceField)
{
}

const char *BTMonsterChanceEditor::monsterChanceDescription[FIELDS_MONSTERCHANCE] = { "Chance", "Groups"};
const char *BTMonsterChanceEditor::monsterChanceField[FIELDS_MONSTERCHANCE] = { "chance", "groups"};

BTItemEditor::BTItemEditor()
 : BTSerializedEditor(FIELDS_ITEM, itemDescription, itemField)
{
}

const char *BTItemEditor::itemDescription[FIELDS_ITEM] = { "Name", "Type", "User Class", "Price", "Armor Plus", "Hit Plus", "Damage Dice", "X-Special", "Likelihood of X-Special", "Times Usable", "Consumed", "Spell Cast", "Cause", "Effect" };
const char *BTItemEditor::itemField[FIELDS_ITEM] = { "name", "type", "allowedJob", "price", "armorPlus", "hitPlus", "damage", "xSpecial", "chanceXSpecial", "timesUsable", "consume", "spell", "cause", "effect" };

#define MONSTERLOC_RANGEDTYPE 19
#define MONSTERLOC_RANGEDSPELL 20

BTMonsterEditor::BTMonsterEditor()
 : BTSerializedEditor(FIELDS_MONSTER, monsterDescription, monsterField, true)
{
}


void BTMonsterEditor::initActive(ObjectSerializer &serial, BitField &active)
{
 BTFactory<BTMonster> &monsterList = BTCore::getCore()->getMonsterList();
 prevXp = monsterList[current].calcXp();
 XMLAction *curField = serial.find("rangedType", NULL);
 int type = *(reinterpret_cast<int*>(curField->object));
 for (int i = 0; i < entries; ++i)
 {
  switch (i)
  {
   case MONSTERLOC_RANGEDSPELL:
   {
    if (type == BTRANGEDTYPE_MAGIC)
     active.set(i);
    else
     active.clear(i);
    break;
   }
   default:
    active.set(i);
  }
 }
}

bool BTMonsterEditor::updateActive(ObjectSerializer &serial, BitField &active, int modField)
{
 BTFactory<BTMonster> &monsterList = BTCore::getCore()->getMonsterList();
 bool refresh = false;
 unsigned int newXp = monsterList[current].calcXp();
 if (newXp != prevXp)
 {
  XMLAction *xpField = serial.find("xp", NULL);
  unsigned int *xp = (reinterpret_cast<unsigned int*>(xpField->object));
  *xp += newXp;
  *xp -= prevXp;
  prevXp = newXp;
  refresh = true;
 }
 if (modField == MONSTERLOC_RANGEDTYPE)
 {
  BitField old = active;
  XMLAction *curField = serial.find("rangedType", NULL);
  int type = *(reinterpret_cast<int*>(curField->object));
  if (type == BTRANGEDTYPE_MAGIC)
  {
   if (!active.isSet(MONSTERLOC_RANGEDSPELL))
   {
    active.set(MONSTERLOC_RANGEDSPELL);
    XMLAction *extraField = serial.find("rangedSpell", NULL);
    *(reinterpret_cast<int*>(extraField->object)) = 0;
   }
  }
  else
   active.clear(MONSTERLOC_RANGEDSPELL);
  return (refresh || (!(active == old)));
 }
 return refresh;
}

const char *BTMonsterEditor::monsterDescription[FIELDS_MONSTER] = { "Name", "Plural", "Illusion", "Picture", "Gender", "Level", "Starting Distance", "Moves Per Round", "Rate of Attacks", "Base AC", "Upper Limit Appearing", "Hit Points", "Thaumaturigal Resistance", "Gold", "Wandering", "Combat Actions", "Attack Msg.", "Damage", "Extra Damage", "Ranged Type", "Ranged Spell", "Ranged Message", "Range", "Ranged Damage", "Ranged X-Damage", "XP" };
const char *BTMonsterEditor::monsterField[FIELDS_MONSTER] = { "name", "pluralName", "illusion", "picture", "gender", "level", "startDistance", "move", "rateAttacks", "ac", "maxAppearing", "hp", "magicResistance", "gold", "wandering", "combatAction", "meleeMessage", "meleeDamage", "meleeExtra", "rangedType", "rangedSpellName", "rangedMessage", "range", "rangedDamage", "rangedExtra", "xp" };

BTSpellEditor::BTSpellEditor()
 : BTSerializedEditor(FIELDS_SPELL, spellDescription, spellField, true)
{
}

void BTSpellEditor::complete(ObjectSerializer &serial)
{
 XMLAction *rangeField = serial.find("range", NULL);
 XMLAction *effectiveRangeField = serial.find("effectiveRange", NULL);
 IShort range = *(reinterpret_cast<IShort*>(rangeField->object));
 IShort effectiveRange = *(reinterpret_cast<IShort*>(effectiveRangeField->object));
 XMLAction *manifestField = serial.find("manifest", NULL);
 XMLArray *manifestArray = (reinterpret_cast<XMLArray*>(manifestField->object));
 XMLVector<BTManifest*> *manifest = dynamic_cast<XMLVector<BTManifest*> *>(manifestArray);
 for (int i = 0; i < manifest->size(); ++i)
 {
  BTRangedManifest *rangedManifest = dynamic_cast<BTRangedManifest*>(manifest->get(i));
  if (rangedManifest)
  {
   rangedManifest->range = range;
   rangedManifest->effectiveRange = effectiveRange;
  }
 }
}

void BTSpellEditor::delSpecialField(BTDisplay &d, ObjectSerializer &serial, int val)
{
 val -= 100;
 if ((val != extra.size()) && (extra[val].name == "type"))
 {
  XMLAction *manifestField = serial.find("manifest", NULL);
  XMLArray *manifestArray = (reinterpret_cast<XMLArray*>(manifestField->object));
  XMLVector<BTManifest*> *manifest = dynamic_cast<XMLVector<BTManifest*> *>(manifestArray);
  for (int i = 0; i < manifest->size(); ++i)
  {
   if (extra[val].item == (*manifest)[i])
   {
    manifest->erase(i);
    break;
   }
  }
 }
}

void BTSpellEditor::handleSpecialField(BTDisplay &d, ObjectSerializer &serial, int val)
{
 val -= 100;
 if ((val == extra.size()) || (extra[val].name == "type"))
 {
  BTDisplay::selectItem lookupItem[BT_SPELLTYPES_USED];
  int lookupStart(0);
  int lookupCurrent(0);
  for (int i = 0; i < BT_SPELLTYPES_USED; ++i)
  {
   lookupItem[i].name = spellTypeLookup.getName(spellTypes[i].type);
   if ((val != extra.size()) && (extra[val].item->type == spellTypes[i].type))
    lookupCurrent = i;
  }
  d.addSelection(lookupItem, BT_SPELLTYPES_USED, lookupStart, lookupCurrent);
  if ((27 != d.process()) && ((val == extra.size()) || (lookupCurrent != extra[val].item->type)))
  {
   XMLAction *manifestField = serial.find("manifest", NULL);
   XMLArray *manifestArray = (reinterpret_cast<XMLArray*>(manifestField->object));
   XMLVector<BTManifest*> *manifest = dynamic_cast<XMLVector<BTManifest*> *>(manifestArray);
   if (val == extra.size())
   {
    manifest->push_back((*spellTypes[lookupCurrent].f)(NULL, NULL));
    (*manifest)[manifest->size()  - 1]->type = spellTypes[lookupCurrent].type;
   }
   else
   {
    for (int i = 0; i < manifest->size(); ++i)
    {
     if (extra[val].item == (*manifest)[i])
     {
      (*manifest)[i] = dynamic_cast<BTManifest*>((*spellTypes[lookupCurrent].f)(NULL, NULL));
      (*manifest)[i]->type = spellTypes[lookupCurrent].type;
      delete extra[val].item;
      break;
     }
    }
   }
  }
  d.clearText();
 }
 else
 {
  ObjectSerializer serialSub;
  extra[val].item->serialize(&serialSub);
  XMLAction *manifestField = serialSub.find(extra[val].name.c_str(), NULL);
  editField(d, serialSub, extra[val].item->getEditFieldDescription(extra[val].value), manifestField, extra[val].value, 0);
 }
}

int BTSpellEditor::setup(ObjectSerializer &serial, BitField &active, std::vector<BTDisplay::selectItem> &items)
{
 int current = BTSerializedEditor::setup(serial, active, items);
 XMLAction *manifestField = serial.find("manifest", NULL);
 XMLArray *manifestArray = (reinterpret_cast<XMLArray*>(manifestField->object));
 XMLVector<BTManifest*> *manifest = dynamic_cast<XMLVector<BTManifest*> *>(manifestArray);
 extra.clear();
 int extraVal = 100;
 char convert[50];
 for (int i = 0; i < manifest->size(); ++i)
 {
  if (current == items.size())
   items.push_back(BTDisplay::selectItem());
  BTManifest *manifestObj = dynamic_cast<BTManifest*>(manifest->get(i));
  extra.push_back(BTSpellEditor::extraItems(manifestObj, "type", -1));
  items[current].name = std::string("Effect: ") + spellTypeLookup.getName(manifestObj->type);
  items[current].value = extraVal++;
  current++;
  int manifestEntries = manifestObj->getEditFieldNumber();
  if (manifestEntries > 0)
  {
   ObjectSerializer serialSub;
   manifestObj->serialize(&serialSub);
   for (int n = 0; n < manifestEntries; ++n)
   {
    if (current == items.size())
     items.push_back(BTDisplay::selectItem());
    extra.push_back(BTSpellEditor::extraItems(manifestObj, manifestObj->getEditField(n), n));
    XMLAction *curField = serialSub.find(manifestObj->getEditField(n), NULL);
    items[current].name = std::string("  ") + std::string(manifestObj->getEditFieldDescription(n)) + ": " + curField->createString();
    items[current].value = extraVal++;
    current++;
   }
  }
 }
 if (current == items.size())
  items.push_back(BTDisplay::selectItem());
 items[current].name = std::string("<New Effect>");
 items[current].value = extraVal++;
 current++;
 return current;
}

bool BTSpellEditor::updateActive(ObjectSerializer &serial, BitField &active, int modField)
{
 return ((modField < 100) ? false : true);
}

BTSpellEditor::spellType BTSpellEditor::spellTypes[BT_SPELLTYPES_USED] =
{
 {BTSPELLTYPE_HEAL, BTHealManifest::create},
 {BTSPELLTYPE_RESURRECT, BTResurrectManifest::create},
 {BTSPELLTYPE_DAMAGE, BTAttackManifest::create},
 {BTSPELLTYPE_SUMMONMONSTER, BTSummonManifest::create},
 {BTSPELLTYPE_SUMMONILLUSION, BTSummonManifest::create},
 {BTSPELLTYPE_DISPELLILLUSION, BTRangedManifest::create},
 {BTSPELLTYPE_SCRYSIGHT, BTScrySightManifest::create},
 {BTSPELLTYPE_LIGHT, BTLightManifest::create},
 {BTSPELLTYPE_ARMORBONUS, BTBonusManifest::create},
 {BTSPELLTYPE_HITBONUS, BTBonusManifest::create},
 {BTSPELLTYPE_TRAPDESTROY, BTManifest::create},
 {BTSPELLTYPE_DOORDETECT, BTManifest::create},
 {BTSPELLTYPE_PHASEDOOR, BTPhaseDoorManifest::create},
 {BTSPELLTYPE_DISPELLMAGIC, BTRangedManifest::create},
 {BTSPELLTYPE_COMPASS, BTManifest::create},
 {BTSPELLTYPE_PUSH, BTPushManifest::create},
 {BTSPELLTYPE_ATTACKRATEBONUS, BTBonusManifest::create},
 {BTSPELLTYPE_REGENMANA, BTRegenManaManifest::create},
 {BTSPELLTYPE_SAVEBONUS, BTBonusManifest::create},
 {BTSPELLTYPE_BLOCKENCOUNTERS, BTManifest::create},
 {BTSPELLTYPE_BLOCKMAGIC, BTTargetedManifest::create},
 {BTSPELLTYPE_SPELLBIND, BTSpellBindManifest::create},
 {BTSPELLTYPE_LEVITATION, BTManifest::create},
 {BTSPELLTYPE_REGENSKILL, BTRegenSkillManifest::create},
 {BTSPELLTYPE_CURESTATUS, BTCureStatusManifest::create}
};
const char *BTSpellEditor::spellDescription[FIELDS_SPELL] = { "Name", "Code", "Mage Class", "Level", "Points Needed", "Range", "Effective Range", "Area/Target", "Duration", "Effect Text" };
const char *BTSpellEditor::spellField[FIELDS_SPELL] = { "name", "code", "caster", "level", "sp", "range", "effectiveRange", "area", "duration", "effect" };

