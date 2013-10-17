/*-------------------------------------------------------------------------*\
  <editor.C> -- Editor implementation file

  Date      Programmer  Description
  10/10/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "editor.h"
#include <algorithm>
#include <sstream>

BTEditor::BTEditor(BTModule *m)
 : BTCore(m), currentWall(0), startSpecial(0), currentSpecial(0)
{
}

BTEditor::~BTEditor()
{
}

int BTEditor::getLight()
{
 return 5;
}

int BTEditor::getWallType(int x, int y, int direction)
{
 if (x < 0)
  x += levelMap->getXSize();
 x = x % levelMap->getXSize();
 if (y < 0)
  y += levelMap->getYSize();
 y = y % levelMap->getYSize();
 IShort w = levelMap->getSquare(y, x).getWall(direction);
 if (w == 0)
  return 0;
 int mapType = p3dConfig->findMapType(w, true);
 if (mapType == 0)
  return 0;
 if (-1 != p3dConfig->mapType[mapType - 1]->incompleteType)
  return p3dConfig->mapType[mapType - 1]->incompleteType;
 return w;
}

int BTEditor::getKnowledge(int x, int y)
{
 return BTKNOWLEDGE_FULL;
}

void BTEditor::edit(BTDisplay &d)
{
 char **files = PHYSFS_enumerateFiles("");
 char **i;
 int count(0);

 for (i = files; *i != NULL; i++)
 {
  if ((0 == strcmp("shops.xml", *i)) || (0 == strcmp("roster.xml", *i)))
   continue;
  int len = strlen(*i);
  if ((len > 4) && (strcmp(".MAP", (*i) + (len - 4)) == 0))
  {
   char tmp[len + 1];
   strcpy(tmp, (*i));
   strcpy(tmp + len - 3, "xml");
   if (0 == PHYSFS_exists(tmp))
   {
    count++;
   }
  }
  else if ((len > 4) && (strcmp(".xml", (*i) + (len - 4)) == 0))
  {
   count++;
  }
 }
 BTDisplay::selectItem *list = new BTDisplay::selectItem[count];
 list[0].name = module->item;
 list[1].name = module->spell;
 list[2].name = module->monster;
 int current = 3;
 for (i = files; *i != NULL; i++)
 {
  if ((0 == strcmp(module->monster, *i)) || (0 == strcmp(module->item, *i)) || (0 == strcmp(module->spell, *i)))
   continue;
  if ((0 == strcmp("shops.xml", *i)) || (0 == strcmp("roster.xml", *i)))
   continue;
  int len = strlen(*i);
  if ((len > 4) && (strcmp(".MAP", (*i) + (len - 4)) == 0))
  {
   char tmp[len + 1];
   strcpy(tmp, (*i));
   strcpy(tmp + len - 3, "xml");
   if (0 == PHYSFS_exists(tmp))
   {
    list[current].name = *i;
    current++;
   }
  }
  else if ((len > 4) && (strcmp(".xml", (*i) + (len - 4)) == 0))
  {
   list[current].name = *i;
   current++;
  }
 }
 PHYSFS_freeList(files);
 int start(0);
 int select(0);
 d.clearElements();
 d.addSelection(list, count, start, select);
 unsigned int key = d.process();
 d.clearText();
 if (key == 13)
 {
  if (list[select].name == module->monster)
  {
   BTFactory<BTMonster> &monsterList = getMonsterList();
   int monster = 0;
   while (-1 != (monster = editFactoryList<BTMonster>(d, monsterList, "<New Monster>")))
   {
    editMonster(d, monsterList[monster]);
   }
   monsterList.save(module->monster);
  }
  else if (list[select].name == module->spell)
  {
   BTFactory<BTSpell> &spellList = getSpellList();
   int spell = 0;
   while (-1 != (spell = editFactoryList<BTSpell>(d, spellList, "<New Spell>")))
   {
    editSpell(d, spellList[spell]);
   }
   spellList.save(module->spell);
  }
  else if (list[select].name == module->item)
  {
   BTFactory<BTItem> &itemList = getItemList();
   int item = 0;
   while (-1 != (item = editFactoryList<BTItem>(d, itemList, "<New Item>")))
   {
    editItem(d, itemList[item]);
   }
   itemList.save(module->item);
  }
  else
   editMap(d, list[select].name.c_str());
 }
}

void BTEditor::editMap(BTDisplay &d, const char *filename)
{
 BTDisplayConfig *oldConfig = d.getConfig();
 BTDisplayConfig config;
 XMLSerializer parser;
 config.serialize(&parser);
 parser.parse("data/mapedit.xml", true);
 d.setConfig(&config);
 d.setPsuedo3DConfig(module->wall);
 loadMap(filename);
 xPos = 0; yPos = 0; facing = 0;
 p3dConfig = d.setWallGraphics(levelMap->getType());
 unsigned char key = ' ';
 if (currentWall < p3dConfig->mapType.size())
  d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
 else
  d.drawText("Clear");
 while (key != 'q')
 {
  if (levelMap->getSquare(yPos, xPos).getSpecial() > -1)
   d.drawLabel(levelMap->getSpecial(levelMap->getSquare(yPos, xPos).getSpecial())->getName());
  else
   d.drawLabel("");
  d.drawView();
  key = d.readChar();
  switch (key)
  {
   case BTKEY_UP:
    if (yPos > 0)
     yPos--;
    else
     yPos = getYSize() - 1;
    break;
   case BTKEY_LEFT:
    if (xPos > 0)
     xPos--;
    else
     xPos = getXSize() - 1;
    break;
   case BTKEY_DOWN:
    if (yPos < getYSize() - 1)
     yPos++;
    else
     yPos = 0;
    break;
   case BTKEY_RIGHT:
    if (xPos < getXSize() - 1)
     xPos++;
    else
     xPos = 0;
    break;
   case BTKEY_PGDN:
    if (facing < 3)
     facing++;
    else
     facing = 0;
    break;
   case BTKEY_END:
    if (facing > 0)
     facing--;
    else
     facing = 3;
    break;
   case 13:
   {
    int wall = 0;
    if (currentWall < p3dConfig->mapType.size())
     wall = p3dConfig->mapType[currentWall]->type;
    levelMap->getSquare(yPos, xPos).setWall(facing, wall);
    int xOpposite = xPos + Psuedo3D::changeXY[facing][0] + levelMap->getXSize();
    xOpposite = xOpposite % levelMap->getXSize();
    int yOpposite = yPos + Psuedo3D::changeXY[facing][1] + levelMap->getYSize();
    yOpposite = yOpposite % levelMap->getYSize();
    levelMap->getSquare(yOpposite, xOpposite).setWall((facing + 2) % 4, wall);
    break;
   }
   case ' ':
    if (currentWall < p3dConfig->mapType.size())
    {
     currentWall++;
    }
    else
    {
     currentWall = 0;
    }
    if (currentWall < p3dConfig->mapType.size())
     d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
    else
     d.drawText("Clear");
    break;
   case 'r':
   {
    std::string tmp = d.readString("X Size?", 3, "");
    int newXSize = atol(tmp.c_str());
    if (newXSize < 1)
     break;
    tmp = d.readString("Y Size?", 3, "");
    int newYSize = atol(tmp.c_str());
    if (newYSize < 1)
     break;
    levelMap->resize(newXSize, newYSize);
    break;
   }
   case 'c':
    levelMap->getSquare(yPos, xPos).setSpecial(-1);
    break;
   case 's':
   {
    d.clearText();
    int len = levelMap->getNumOfSpecials();
    BTDisplay::selectItem list[len + 1];
    for (int i = 0; i < len; ++i)
    {
     list[i].name = levelMap->getSpecial(i)->getName();
    }
    list[len].name = "<New Special>";
    d.addSelection(list, len + 1, startSpecial, currentSpecial);
    int key = d.process("e");
    d.clearText();
    if ((key == 'e') || ((currentSpecial == len) && (key == '\r')))
    {
     editSpecial(d, levelMap->getSpecial(currentSpecial));
     levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
    }
    else if (key == '\r')
    {
     levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
    }
    if (currentWall < p3dConfig->mapType.size())
     d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
    else
     d.drawText("Clear");
    break;
   }
   case 'l':
    levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
    break;
   case 'p':
    editMapProperties(d);
    break;
   default:
    break;
  }
 }
 d.drawText("Save?");
 while ((key != 'y') && (key != 'n'))
 {
  key = d.readChar();
 }
 if (key == 'y')
 {
  bool wrote = false;
  int len = strlen(levelMap->getFilename());
  if ((len > 4) && (strcmp(".MAP", levelMap->getFilename() + (len - 4)) == 0))
  {
   try
   {
    BinaryWriteFile f(levelMap->getFilename());
    levelMap->write(f);
    wrote = true;
   }
   catch (const FileException &e)
   {
    PHYSFS_delete(levelMap->getFilename());
    printf("Failed to write old map file: %s\n", e.what());
    char tmp[len + 1];
    strcpy(tmp, levelMap->getFilename());
    strcpy(tmp + len - 3, "xml");
    levelMap->setFilename(tmp);
   }
  }
  if (!wrote)
  {
   XMLSerializer parser;
   parser.add("map", levelMap);
   parser.write(levelMap->getFilename(), true);
  }
 }
 d.setConfig(oldConfig);
}

#define FIELDS_MAP 4

void BTEditor::editMapProperties(BTDisplay &d)
{
 const char *description[FIELDS_MAP] = { "Name", "Type", "Light", "Monster Level" };
 const char *field[FIELDS_MAP] = { "name", "type", "light", "monsterLevel" };
 ObjectSerializer serial;
 levelMap->serialize(&serial);
 editSerialized(d, serial, FIELDS_MAP, description, field);
}

void BTEditor::editSpecial(BTDisplay &d, BTSpecial *special)
{
 if (NULL == special)
 {
  special = new BTSpecial;
  levelMap->addSpecial(special);
 }
 BTDisplayConfig *oldConfig = d.getConfig();
 BTDisplayConfig config;
 XMLSerializer parser;
 config.serialize(&parser);
 parser.parse("data/specialedit.xml", true);
 d.setConfig(&config);
 int start(0);
 int current(0);
 BTSpecialBody *body = special->getBody();
 std::vector<operationList> ops;
 std::vector<BTDisplay::selectItem> list(2);
 list[0].name = std::string("Name: ") + special->getName();
 list[1].name = "Flags: " + special->printFlags();
 int spaces = 0;
 buildOperationList(body, list, ops);
 d.addSelection(list.data(), list.size(), start, current);
 int key;
 char extra[3] = {BTKEY_INS, BTKEY_DEL, 0};
 while (27 != (key = d.process(extra)))
 {
  d.clearText();
  if (current == 0)
  {
   std::string name = special->getName();
   d.addReadString("Name: ", 25, name);
   key = d.process();
   if ('\r' == key)
    special->setName(name);
   d.clearText();
   list[0].name = std::string("Name: ") + special->getName();
  }
  else if (current == 1)
  {
  }
  else
  {
   if (BTKEY_INS == key)
   {
    if ((ops[current - 2].op != NULL) && (ops[current - 2].parent != NULL))
    {
     ops[current - 2].parent->insertOperation(ops[current - 2].op, new BTSpecialCommand(BTSPECIALCOMMAND_NOTHING));
    }
   }
   else if (BTKEY_DEL == key)
   {
    if ((ops[current - 2].op != NULL) && (ops[current - 2].parent != NULL))
    {
     ops[current - 2].parent->eraseOperation(ops[current - 2].op);
    }
   }
   else if ('\r' == key)
   {
    BTSpecialOperation *op = editSpecialOperation(d, ops[current - 2].op);
    if (op)
    {
     if (ops[current - 2].op)
      ops[current - 2].parent->replaceOperation(ops[current - 2].op, op);
     else
      ops[current - 2].parent->addOperation(op);
    }
   }
  }
  ops.clear();
  list.resize(2);
  buildOperationList(body, list, ops);
  d.addSelection(list.data(), list.size(), start, current);
 }
 d.clearText();
 d.setConfig(oldConfig);
}

#define FIELDS_ITEM 14
#define FIELDS_MONSTER 14
#define FIELDS_SPELL 5

void BTEditor::editItem(BTDisplay &d, BTItem &item)
{
 const char *description[FIELDS_ITEM] = { "Name", "Type", "User Class", "Price", "Armor Plus", "Hit Plus", "Damage Dice", "X-Special", "Likelihood of X-Special", "Times Usable", "Consumed", "Spell Cast", "Cause", "Effect" };
 const char *field[FIELDS_ITEM] = { "name", "type", "allowedJob", "price", "armorPlus", "hitPlus", "damage", "xSpecial", "chanceXSpecial", "timesUsable", "consume", "spell", "cause", "effect" };
 ObjectSerializer serial;
 item.serialize(&serial);
 editSerialized(d, serial, FIELDS_ITEM, description, field);
}

void BTEditor::editMonster(BTDisplay &d, BTMonster &monster)
{
 const char *description[FIELDS_MONSTER] = { "Name", "Plural", "Picture", "Gender", "Level", "Starting Distance", "Moves Per Round", "Rate of Attacks", "Base AC", "Upper Limit Appearing", "Hit Points", "Thaumaturigal Resistance", "Gold", "Wandering" };
 const char *field[FIELDS_MONSTER] = { "name", "pluralName", "picture", "gender", "level", "startDistance", "move", "rateAttacks", "ac", "maxAppearing", "hp", "magicResistance", "gold", "wandering" };
 ObjectSerializer serial;
 monster.serialize(&serial);
 editSerialized(d, serial, FIELDS_MONSTER, description, field);
}

void BTEditor::editSpell(BTDisplay &d, BTSpell &spell)
{
 const char *description[FIELDS_SPELL] = { "Name", "Code", "Mage Class", "Level", "Points Needed" };
 const char *field[FIELDS_SPELL] = { "name", "code", "caster", "level", "sp" };
 ObjectSerializer serial;
 spell.serialize(&serial);
 editSerialized(d, serial, FIELDS_SPELL, description, field);
}

void BTEditor::editSerialized(BTDisplay &d, ObjectSerializer &serial, int entries, const char *description[], const char *field[])
{
 BTDisplayConfig *oldConfig = d.getConfig();
 BTDisplayConfig config;
 XMLSerializer parser;
 config.serialize(&parser);
 parser.parse("data/specialedit.xml", true);
 d.setConfig(&config);
 int start(0);
 int current(0);
 std::vector<BTDisplay::selectItem> list(entries);
 for (int i = 0; i < entries; ++i)
 {
  XMLAction *curField = serial.find(field[i], NULL);
  if (curField->getType() == XMLTYPE_OBJECT)
  {
   XMLObject *obj = reinterpret_cast<XMLObject*>(curField->object);
   BTDice *dice = dynamic_cast<BTDice*>(obj);
   if (dice)
    list[i].name = std::string(description[i]) + ": " + dice->createString();
  }
  else
  {
   list[i].name = std::string(description[i]) + ": " + curField->createString();
  }
 }
 d.addSelection(list.data(), list.size(), start, current);
 int key;
 while (27 != (key = d.process()))
 {
  d.clearText();
  XMLAction *curField = serial.find(field[current], NULL);
  if (curField)
  {
   switch (curField->getType())
   {
    case XMLTYPE_STDSTRING:
    {
     std::string val = curField->createString();
     d.addReadString(std::string(description[current]) + ": ", 100, val);
     key = d.process();
     if ('\r' == key)
      *(reinterpret_cast<std::string*>(curField->object)) = val;
     break;
    }
    case XMLTYPE_STRING:
    {
     std::string val = curField->createString();
     d.addReadString(std::string(description[current]) + ": ", 100, val);
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
      if (extra)
       lookupItem[0].name = curField->extraText;
      for (int i = 0; i < lookup->size(); ++i)
       lookupItem[i + (extra ? 1 : 0)].name = lookup->getName(i);
      int lookupStart(0);
      int lookupCurrent((*(reinterpret_cast<int*>(curField->object))) + (extra ? 1 : 0));
      d.addSelection(lookupItem, lookup->size() + (extra ? 1 : 0), lookupStart, lookupCurrent);
      if (27 != d.process())
      {
       *(reinterpret_cast<int*>(curField->object)) = lookupCurrent - (extra ? 1 : 0);
      }
     }
     else
     {
      std::string val = curField->createString();
      d.addReadString(std::string(description[current]) + ": ", 100, val);
      key = d.process();
      if ('\r' == key)
       *(reinterpret_cast<int*>(curField->object)) = atol(val.c_str());
     }
     break;
    }
    case XMLTYPE_INT16:
    {
     std::string val = curField->createString();
     d.addReadString(std::string(description[current]) + ": ", 100, val);
     key = d.process();
     if ('\r' == key)
      *(reinterpret_cast<int16_t*>(curField->object)) = atol(val.c_str());
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
      d.addReadString(std::string(description[current]) + "- Number of Dice: ", 100, val);
      key = d.process();
      if ('\r' == key)
       dice->setNumber(atol(val.c_str()));
      d.clearText();
      stream.str("");
      stream << dice->getType();
      val = stream.str();
      d.addReadString(std::string(description[current]) + "- Type of Dice: ", 100, val);
      key = d.process();
      if ('\r' == key)
       dice->setType(atol(val.c_str()));
      d.clearText();
      stream.str("");
      stream << dice->getModifier();
      val = stream.str();
      d.addReadString(std::string(description[current]) + "- Modifier to Roll: ", 100, val);
      key = d.process();
      if ('\r' == key)
       dice->setModifier(atol(val.c_str()));
     }
     else
      printf("Unsuppported type: %d\n", curField->getType());
     break;
    }
    default:
     printf("Unsuppported type: %d\n", curField->getType());
     break;
   }
   d.clearText();
   if (curField->getType() == XMLTYPE_OBJECT)
   {
    XMLObject *obj = reinterpret_cast<XMLObject*>(curField->object);
    BTDice *dice = dynamic_cast<BTDice*>(obj);
    if (dice)
     list[current].name = std::string(description[current]) + ": " + dice->createString();
   }
   else
   {
    list[current].name = std::string(description[current]) + ": " + curField->createString();
   }
  }
  d.addSelection(list.data(), list.size(), start, current);
 }
 d.clearText();
 d.setConfig(oldConfig);
}

void BTEditor::buildOperationList(BTSpecialBody *body, std::vector<BTDisplay::selectItem> &list, std::vector<operationList> &ops, int level /*= 0*/)
{
 std::string spaces(level, ' ');
 for (int i = 0; i < body->numOfOperations(false); ++i)
 {
  BTSpecialOperation *op = body->getOperation(i);
  if (level == 0)
  {
   list.push_back(BTDisplay::selectItem(spaces + op->print()));
  }
  else
   list.push_back(BTDisplay::selectItem(spaces + op->print()));
  ops.push_back(operationList(body, op));
  BTSpecialBody *subBody = dynamic_cast<BTSpecialBody*>(op);
  if (subBody)
  {
   buildOperationList(subBody, list, ops, level + 1);
  }
  else
  {
   BTSpecialConditional *conditional = dynamic_cast<BTSpecialConditional*>(op);
   if (conditional)
   {
    buildOperationList(conditional->getThenClause(), list, ops, level + 1);
    list.push_back(BTDisplay::selectItem(spaces + "ELSE"));
    list.back().flags.set(BTSELECTFLAG_UNSELECTABLE);
    ops.push_back(operationList(NULL, NULL));
    buildOperationList(conditional->getElseClause(), list, ops, level + 1);
   }
  }
 }
 list.push_back(BTDisplay::selectItem(spaces + "<New Operation>"));
 ops.push_back(operationList(body, NULL));
}

BTSpecialOperation *BTEditor::editSpecialOperation(BTDisplay &d, BTSpecialOperation *special)
{
 BTDisplay::selectItem cmds[BT_SPECIALCOMMANDS + BT_CONDITIONALCOMMANDS];
 for (int i = 0; i < BT_CONDITIONALCOMMANDS; ++i)
 {
  cmds[i].name = std::string("if ") + conditionalCommands[i];
  cmds[i].value = i;
 }
 for (int i = 0; i < BT_SPECIALCOMMANDS; ++i)
 {
  cmds[i + BT_CONDITIONALCOMMANDS].name = specialCommands[i];
  cmds[i + BT_CONDITIONALCOMMANDS].value = i + BT_CONDITIONALCOMMANDS;
 }
 std::sort(cmds, cmds + BT_SPECIALCOMMANDS + BT_CONDITIONALCOMMANDS);
 int start(0);
 int current(0);
 d.addSelection(cmds, BT_SPECIALCOMMANDS + BT_CONDITIONALCOMMANDS, start, current);
 int key = d.process();
 d.clearText();
 if (key == 27)
  return NULL;
 std::string text;
 int number[3] = {0, 0, 0};
 int count = 0;
 const char *cmd = NULL;
 if (cmds[current].value < BT_CONDITIONALCOMMANDS)
 {
  cmd = conditionalCommands[cmds[current].value];
 }
 else
 {
  cmd = specialCommands[cmds[current].value - BT_CONDITIONALCOMMANDS];
 }
 const char *dollarSign;
 while (dollarSign = strchr(cmd, '$'))
 {
  switch (dollarSign[1])
  {
   case 'S':
   {
    int len = levelMap->getNumOfSpecials();
    BTDisplay::selectItem list[len];
    for (int i = 0; i < len; ++i)
    {
     list[i].name = levelMap->getSpecial(i)->getName();
    }
    int specialStart(0);
    d.addSelection(list, len, specialStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'I':
   {
    BTFactory<BTItem> &itemList = getItemList();
    BTDisplay::selectItem items[itemList.size()];
    for (int i = 0; i < itemList.size(); ++i)
     items[i].name = itemList[i].getName();
    int itemStart(0);
    d.addSelection(items, itemList.size(), itemStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'A':
   case 'M':
   {
    BTFactory<BTMonster> &monsterList = getMonsterList();
    BTDisplay::selectItem monsters[monsterList.size()];
    for (int i = 0; i < monsterList.size(); ++i)
     monsters[i].name = monsterList[i].getName();
    int monsterStart(0);
    d.addSelection(monsters, monsterList.size(), monsterStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'X':
   {
    BTFactory<BTSpell> &spellList = getSpellList();
    BTDisplay::selectItem spells[spellList.size()];
    for (int i = 0; i < spellList.size(); ++i)
     spells[i].name = spellList[i].getName();
    int spellStart(0);
    d.addSelection(spells, spellList.size(), spellStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'L':
   {
    std::string val;
    d.addReadString("X>", 100, val);
    key = d.process();
    d.clearText();
    if (27 == key)
     return NULL;
    number[count++] = atol(val.c_str());
    val = "";
    d.addReadString("Y>", 100, val);
    key = d.process();
    d.clearText();
    if (27 == key)
     return NULL;
    number[count++] = atol(val.c_str());
    break;
   }
   case 'T':
   {
    BTDisplay::selectItem damage[BT_MONSTEREXTRADAMAGE];
    for (int i = 0; i < BT_MONSTEREXTRADAMAGE; ++i)
     damage[i].name = extraDamage[i];
    int damageStart(0);
    d.addSelection(damage, BT_MONSTEREXTRADAMAGE, damageStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'C':
   {
    BTJobList &jobList = getJobList();
    BTDisplay::selectItem jobs[jobList.size()];
    for (int i = 0; i < jobList.size(); ++i)
     jobs[i].name = jobList[i]->name;
    int jobStart(0);
    d.addSelection(jobs, jobList.size(), jobStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'R':
   {
    BTRaceList &raceList = getRaceList();
    BTDisplay::selectItem races[raceList.size()];
    for (int i = 0; i < raceList.size(); ++i)
     races[i].name = raceList[i]->name;
    int raceStart(0);
    d.addSelection(races, raceList.size(), raceStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case 'D':
   {
    BTDisplay::selectItem dir[BT_DIRECTIONS];
    for (int i = 0; i < BT_DIRECTIONS; ++i)
     dir[i].name = directions[i];
    int dirStart(0);
    d.addSelection(dir, BT_DIRECTIONS, dirStart, number[count]);
    int key = d.process();
    d.clearText();
    if (key == 27)
     return NULL;
    count++;
    break;
   }
   case '#':
   case 'P':
   case 'G':
   case 'F':
   case '!':
   case 'J':
   {
    std::string val;
    d.addReadString("Number>", 100, val);
    key = d.process();
    d.clearText();
    if (27 == key)
     return NULL;
    number[count++] = atol(val.c_str());
    break;
   }
   case '$':
   case 'N':
   default:
    d.addReadString(">", 100, text);
    key = d.process();
    d.clearText();
    if (27 == key)
     return NULL;
    break;
  }
  cmd = dollarSign + 2;
 }
 if (cmds[current].value < BT_CONDITIONALCOMMANDS)
 {
  BTSpecialConditional *opNew = new BTSpecialConditional(cmds[current].value, text.c_str(), number[0]);
  BTSpecialConditional *opOld = static_cast<BTSpecialConditional*>(special);
  if (opOld)
  {
   opNew->getThenClause()->moveFrom(opOld->getThenClause());
   opNew->getElseClause()->moveFrom(opOld->getElseClause());
  }
  return opNew;
 }
 else
 {
  BTSpecialCommand *opNew = new BTSpecialCommand(cmds[current].value - BT_CONDITIONALCOMMANDS);
  opNew->setText(text);
  for (int i = 0; i < count; ++i)
   opNew->setNumber(i, number[i]);
  return opNew;
 }
}

