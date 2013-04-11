/*-------------------------------------------------------------------------*\
  <editor.C> -- Editor implementation file

  Date      Programmer  Description
  10/10/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "editor.h"

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
  if ((0 == strcmp(module->monster, *i)) || (0 == strcmp(module->item, *i)) || (0 == strcmp(module->spell, *i)))
   continue;
  if ((0 == strcmp("shops.xml", *i)) || (0 == strcmp("roster.xml", *i)))
   continue;
  int len = strlen(*i);
  if ((len > 4) && ((strcmp(".MAP", (*i) + (len - 4)) == 0) || (strcmp(".xml", (*i) + (len - 4)) == 0)))
  {
   count++;
  }
 }
 BTDisplay::selectItem *list = new BTDisplay::selectItem[count];
 int current = 0;
 for (i = files; *i != NULL; i++)
 {
  if ((0 == strcmp(module->monster, *i)) || (0 == strcmp(module->item, *i)) || (0 == strcmp(module->spell, *i)))
   continue;
  if ((0 == strcmp("shops.xml", *i)) || (0 == strcmp("roster.xml", *i)))
   continue;
  int len = strlen(*i);
  if ((len > 4) && ((strcmp(".MAP", (*i) + (len - 4)) == 0) || (strcmp(".xml", (*i) + (len - 4)) == 0)))
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
 if (key == 13)
 {
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
    std::string tmp = d.readString("X Size?", 3);
    int newXSize = atol(tmp.c_str());
    if (newXSize < 1)
     break;
    tmp = d.readString("Y Size?", 3);
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
    BTDisplay::selectItem list[len];
    for (int i = 0; i < len; ++i)
    {
     list[i].name = levelMap->getSpecial(i)->getName();
    }
    d.addSelection(list, len, startSpecial, currentSpecial);
    int key = d.process("e");
    d.clearText();
    if (key == 'e')
    {
     editSpecial(d, levelMap->getSpecial(currentSpecial));
     levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
    }
    else if (key == '\r')
     levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
    if (currentWall < p3dConfig->mapType.size())
     d.drawText(p3dConfig->mapType[currentWall]->name.c_str());
    else
     d.drawText("Clear");
    break;
   }
   case 'l':
    levelMap->getSquare(yPos, xPos).setSpecial(currentSpecial);
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
 std::list<std::string> lines;
 body->print(lines);
 int len = lines.size();
 std::vector<BTDisplay::selectItem> list(len + 2);
 list[0].name = std::string("Name: ") + special->getName();
 list[1].name = "Flags: " + special->printFlags();
 std::list<std::string>::iterator itr(lines.begin());
 for (int i = 0; i < len; ++i, ++itr)
 {
  list[i + 2].name = *itr;
 }
 d.addSelection(list.data(), len + 2, start, current);
 int key;
 while (27 != (key = d.process()))
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
  }
  d.addSelection(list.data(), len + 2, start, current);
 }
 d.clearText();
 d.setConfig(oldConfig);
}

