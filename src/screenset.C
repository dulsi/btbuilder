/*-------------------------------------------------------------------------*\
  <screenset.C> -- Screen set implementation file

  Date      Programmer  Description
  04/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "screenset.h"
#include "game.h"
#include "pc.h"
#include "status.h"

BTElement::BTElement(const char *name, const char **a)
 : isText(false), text(name)
{
 int size = 0;
 if (a)
 {
  const char **att= a;
  for ( ; *att; ++att)
  {
   ++size;
  }
  if (size > 0)
  {
   atts = new char_ptr[size + 1];
   atts[size] = NULL;
   for (int i = 0; i < size; ++i)
   {
    atts[i] = new char[strlen(a[i]) + 1];
    strcpy(atts[i], a[i]);
   }
  }
 }
}

BTElement::~BTElement()
{
 if (atts)
 {
  for (char **att = atts; *att; ++att)
  {
   delete [] *att;
  }
  delete [] atts;
 }
}

void BTBarrier::draw(BTDisplay &d, ObjectSerializer *obj)
{
 d.addBarrier("");
}

BTLine::~BTLine()
{
 for (std::list<std::vector<BTElement*> >::iterator itr(element.begin()); itr != element.end(); ++itr)
 {
  for (int i = 0; i < itr->size(); i++)
   delete (*itr)[i];
 }
}

void BTLine::addCol()
{
 element.push_back(std::vector<BTElement*>());
}

void BTLine::addText(std::string text)
{
 if (element.empty())
 {
  element.push_back(std::vector<BTElement*>());
 }
 element.back().push_back(new BTElement(text));
}

void BTLine::addStat(const char *name, const char **atts)
{
 if (element.empty())
 {
  element.push_back(std::vector<BTElement*>());
 }
 element.back().push_back(new BTElement(name, atts));
}

void BTLine::setAlignment(std::string a)
{
 if (a == "center")
  align = BTDisplay::center;
 else if (a == "right")
  align = BTDisplay::right;
 else
  align = BTDisplay::left;
}

void BTLine::draw(BTDisplay &d, ObjectSerializer *obj)
{
 std::list<std::string> line;
 for (std::list<std::vector<BTElement*> >::iterator itr = element.begin(); itr != element.end(); ++itr)
  line.push_back(eval(*itr, obj));
 if (line.size() == 1)
  d.addText(line.front().c_str(), align);
 else if (line.size() == 0)
  d.addText("");
 else if (line.size() == 2)
  d.add2Column(line.front().c_str(), line.back().c_str());
}

void BTLine::elementData(const XML_Char *name, const XML_Char **atts)
{
 if (0 == strcmp("col", name))
  addCol();
 else
  addStat(name, atts);
}

void BTLine::characterData(const XML_Char *s, int len)
{
 std::string str(s, len);
 addText(str);
}

std::string BTLine::eval(std::vector<BTElement*> &line, ObjectSerializer *obj) const
{
 std::string final;
 for (std::vector<BTElement*>::iterator itr = line.begin(); itr != line.end(); ++itr)
 {
  if ((*itr)->isText)
   final += (*itr)->text;
  else
  {
   XMLAction *state = obj->find((*itr)->text.c_str(), const_cast<const char**>((*itr)->atts));
   if (state)
   {
    switch(state->type)
    {
     case XMLTYPE_BOOL:
      if (*(reinterpret_cast<bool*>(state->object)))
       final += "true";
      else
       final += "false";
      break;
     case XMLTYPE_INT:
      if (state->data)
      {
       final += reinterpret_cast<ValueLookup*>(state->data)->getName(*(reinterpret_cast<int*>(state->object)));
      }
      else
      {
       char tmp[40];
       snprintf(tmp, 40, "%d", *(reinterpret_cast<int*>(state->object)));
       final += tmp;
      }
      break;
     case XMLTYPE_UINT:
     {
      char tmp[40];
      snprintf(tmp, 40, "%u", *(reinterpret_cast<unsigned int*>(state->object)));
      final += tmp;
      break;
     }
     case XMLTYPE_STRING:
      final += *(reinterpret_cast<char**>(state->object));
      break;
     case XMLTYPE_STDSTRING:
      final += *(reinterpret_cast<std::string*>(state->object));
      break;
     case XMLTYPE_BITFIELD:
     default:
      break;
    }
   }
  }
 }
 return final;
}

XMLObject *BTLine::create(const XML_Char *name, const XML_Char **atts)
{
 BTLine *obj = new BTLine();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "align"))
   obj->setAlignment(att[1]);
 }
 return obj;
}

void BTChoice::addCol()
{
}

std::string BTChoice::getKeys()
{
 return keys;
}

std::string BTChoice::getAction()
{
 return action;
}

int BTChoice::getScreen(BTPc *pc)
{
 return screen;
}

void BTChoice::setKeys(std::string k)
{
 keys = k;
}

void BTChoice::setAction(std::string a)
{
 action = a;
}

void BTChoice::setScreen(int s)
{
 screen = s;
}

void BTChoice::draw(BTDisplay &d, ObjectSerializer *obj)
{
 std::string line;
 if (!element.empty())
 {
  line = eval(element.front(), obj);
 }
 d.addChoice(getKeys().c_str(), line.c_str(), align);
}

XMLObject *BTChoice::create(const XML_Char *name, const XML_Char **atts)
{
 BTChoice *obj = new BTChoice();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "align"))
   obj->setAlignment(att[1]);
  else if (0 == strcmp(*att, "keys"))
   obj->setKeys(att[1]);
  else if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
 }
 return obj;
}

void BTReadString::addCol()
{
}

std::string BTReadString::getKeys()
{
 return "\r";
}

std::string BTReadString::getAction()
{
 return action;
}

int BTReadString::getScreen(BTPc *pc)
{
 return screen;
}

void BTReadString::setAction(std::string a)
{
 action = a;
}

void BTReadString::setScreen(int s)
{
 screen = s;
}

void BTReadString::draw(BTDisplay &d, ObjectSerializer *obj)
{
 std::string line;
 if (!element.empty())
 {
  line = eval(element.front(), obj);
 }
 d.addReadString(line.c_str(), 13, response);
}

XMLObject *BTReadString::create(const XML_Char *name, const XML_Char **atts)
{
 BTReadString *obj = new BTReadString();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
 }
 return obj;
}

BTSelectCommon::BTSelectCommon()
 : list(NULL), size(0), start(0), select(0), screen(0), numbered(false)
{
}

BTSelectCommon::~BTSelectCommon()
{
 clear();
}

void BTSelectCommon::clear()
{
 delete [] list;
 list = NULL;
 size = 0;
}

std::string BTSelectCommon::getKeys()
{
 return "\r";
}

std::string BTSelectCommon::getAction()
{
 return action;
}

int BTSelectCommon::getNumber()
{
 return size;
}

int BTSelectCommon::getScreen(BTPc *pc)
{
 return screen;
}

void BTSelectCommon::setAction(std::string a)
{
 action = a;
}

void BTSelectCommon::setScreen(int s)
{
 screen = s;
}

void BTSelectCommon::draw(BTDisplay &d, ObjectSerializer *obj)
{
 if (!list)
 {
  delete [] list;
  list = NULL;
 }
 size = buildList(obj);
 if (select == -1)
 {
  select = 0;
  start = 0;
 }
 if (select >= size)
  select = size - 1;
 if (start >= select)
  start = select;
 d.addSelection(list, size, start, select, (numbered ? getNumber() : 0));
}

int BTSelectRoster::buildList(ObjectSerializer *obj)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 if (0 == roster.size())
  throw BTSpecialError("noroster");
 else if (party.size() >= BT_PARTYSIZE)
  throw BTSpecialError("fullparty");
 list = new BTDisplay::selectItem[roster.size()];
 for (int i = 0; i < roster.size(); ++i)
 {
  list[i].name = roster[i]->name;
 }
 return roster.size();
}

int BTSelectRoster::getScreen(BTPc *pc)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 if ((party.size() >= BT_PARTYSIZE) || (0 == roster.size()))
  return fullscreen;
 else
  return screen;
}

XMLObject *BTSelectRoster::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectRoster *obj = new BTSelectRoster();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
  else if (0 == strcmp(*att, "fullscreen"))
   obj->fullscreen = atoi(att[1]);
 }
 return obj;
}

int BTSelectRace::buildList(ObjectSerializer *obj)
{
 XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
 list = new BTDisplay::selectItem[race.size()];
 for (int i = 0; i < race.size(); ++i)
 {
  list[i].name = race[i]->name;
 }
 return race.size();
}

XMLObject *BTSelectRace::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectRace *obj = new BTSelectRace();
 XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
 obj->numbered = race.size();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
 }
 return obj;
}

int BTSelectJob::buildList(ObjectSerializer *obj)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 XMLAction *act = obj->find("pc", NULL);
 BTPc *pc = static_cast<BTPc*>(reinterpret_cast<XMLObject*>(act->object));
 list = new BTDisplay::selectItem[job.size()];
 int size = 0;
 for (int i = 0; i < job.size(); ++i)
 {
  if (job[i]->isAllowed(pc, true))
  {
   list[size].name = job[i]->name;
   ++size;
  }
 }
 if (numbered)
  numbered = size;
 return size;
}

XMLObject *BTSelectJob::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectJob *obj = new BTSelectJob();
 obj->numbered = 1;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
 }
 return obj;
}

int BTSelectGoods::buildList(ObjectSerializer *obj)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 XMLAction *act = obj->find("pc", NULL);
 BTPc *pc = static_cast<BTPc*>(reinterpret_cast<XMLObject*>(act->object));
 if (pc->isEquipmentFull())
  throw BTSpecialError("fullinventory");
 list = new BTDisplay::selectItem[9];
 for (int i = 0; (i < itemList.size()) && (i < 9); ++i)
 {
  if (!itemList[i].canUse(pc))
   list[i].first = '@';
  list[i].name = itemList[i].getName();
  list[i].value = itemList[i].getPrice();
 }
 return ((9 < itemList.size()) ? 9 : itemList.size());
}

int BTSelectGoods::getScreen(BTPc *pc)
{
 if (pc->isEquipmentFull())
  return fullscreen;
 else
  return screen;
}

XMLObject *BTSelectGoods::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectGoods *obj = new BTSelectGoods();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
  else if (0 == strcmp(*att, "fullscreen"))
   obj->fullscreen = atoi(att[1]);
  else if (0 == strcmp(*att, "shop"))
   obj->shop = atoi(att[1]);
 }
 return obj;
}

BTSelectInventory::BTSelectInventory()
: fullscreen(0), noerror(false), value(false)
{
}

int BTSelectInventory::buildList(ObjectSerializer *obj)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 XMLAction *act = obj->find("pc", NULL);
 BTPc *pc = static_cast<BTPc*>(reinterpret_cast<XMLObject*>(act->object));
 if ((pc->isEquipmentEmpty()) && (!noerror))
  throw BTSpecialError("noinventory");
 list = new BTDisplay::selectItem[BT_ITEMS];
 int len = BT_ITEMS;
 for (int i = 0; i < BT_ITEMS; ++i)
 {
  int id = pc->getItem(i);
  if (id == BTITEM_NONE)
  {
   len = i;
   break;
  }
  if (pc->isEquipped(i))
   list[i].first = '*';
  else if (!itemList[id].canUse(pc))
   list[i].first = '@';
  list[i].name = itemList[id].getName();
  if (value)
   list[i].value = itemList[id].getPrice() / 2;
 }
 return len;
}

int BTSelectInventory::getNumber()
{
 return BT_ITEMS;
}

int BTSelectInventory::getScreen(BTPc *pc)
{
 if (pc->isEquipmentEmpty())
  return fullscreen;
 else
  return screen;
}

XMLObject *BTSelectInventory::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectInventory *obj = new BTSelectInventory();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
  else if (0 == strcmp(*att, "fullscreen"))
   obj->fullscreen = atoi(att[1]);
  else if (0 == strcmp(*att, "noerror"))
   obj->noerror = atoi(att[1]);
  else if (0 == strcmp(*att, "numbered"))
   obj->numbered = atoi(att[1]);
  else if (0 == strcmp(*att, "value"))
   obj->value = atoi(att[1]);
 }
 return obj;
}

std::string BTSelectParty::getKeys()
{
 return "123456789";
}

std::string BTSelectParty::getAction()
{
 return action;
}

int BTSelectParty::getScreen(BTPc *pc)
{
 return screen;
}

void BTSelectParty::draw(BTDisplay &d, ObjectSerializer *obj)
{
 BTParty &party = BTGame::getGame()->getParty();
 if (party.size() == 0)
  throw BTSpecialError("noparty");
 char keys[20];
 for (int i = 0; i < party.size(); ++i)
  keys[i] = i + '1';
 keys[party.size()] = 0;
 d.addBarrier(keys);
}

XMLObject *BTSelectParty::create(const XML_Char *name, const XML_Char **atts)
{
 const char *act = "";
 int s = 0;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   act = att[1];
  else if (0 == strcmp(*att, "screen"))
   s = atoi(att[1]);
 }
 return new BTSelectParty(act, s);
}

BTCan::BTCan(const char *o, char_ptr *a, const char *v)
: option(o), atts(a), checkValue(false), drawn(false)
{
 if (v)
 {
  value = v;
  checkValue = true;
 }
}

BTCan::~BTCan()
{
 if (atts)
 {
  for (char **att = atts; *att; ++att)
  {
   delete [] *att;
  }
  delete [] atts;
 }
}

std::string BTCan::getKeys()
{
 if (drawn)
  return items[0]->getKeys();
 else
  return "";
}

std::string BTCan::getAction()
{
 return items[0]->getAction();
}

int BTCan::getScreen(BTPc *pc)
{
 return items[0]->getScreen(pc);
}

void BTCan::draw(BTDisplay &d, ObjectSerializer *obj)
{
 drawn = false;
 XMLAction *state = obj->find(option.c_str(), const_cast<const char**>(atts));
 if (state)
 {
  switch(state->type)
  {
   case XMLTYPE_BOOL:
    if (false == *(reinterpret_cast<bool*>(state->object)))
     return;
    break;
   case XMLTYPE_INT:
    if (checkValue)
    {
     int v = atoi(value.c_str());
     if (v != *(reinterpret_cast<int*>(state->object)))
      return;
    }
    else if (0 >= *(reinterpret_cast<int*>(state->object)))
     return;
    break;
   case XMLTYPE_UINT:
    if (checkValue)
    {
     unsigned int v = atoi(value.c_str());
     if (v != *(reinterpret_cast<unsigned int*>(state->object)))
      return;
    }
    else if (0 >= *(reinterpret_cast<unsigned int*>(state->object)))
     return;
    break;
   case XMLTYPE_STRING:
   case XMLTYPE_STDSTRING:
     return;
    break;
   case XMLTYPE_BITFIELD:
   default:
    return;
    break;
  }
 }
 else
  return;

 drawn = true;
 for (int i = 0; i < items.size(); ++i)
 {
  items[i]->draw(d, obj);
 }
}

void BTCan::serialize(ObjectSerializer *s)
{
 s->add("barrier", &items, &BTBarrier::create);
 s->add("line", &items, &BTLine::create);
 s->add("choice", &items, &BTChoice::create);
 s->add("readString", &items, &BTReadString::create);
 s->add("selectRoster", &items, &BTSelectRoster::create);
 s->add("selectRace", &items, &BTSelectRace::create);
 s->add("selectJob", &items, &BTSelectJob::create);
 s->add("selectGoods", &items, &BTSelectGoods::create);
 s->add("selectInventory", &items, &BTSelectInventory::create);
 s->add("selectParty", &items, &BTSelectParty::create);
}

XMLObject *BTCan::create(const XML_Char *name, const XML_Char **atts)
{
 const char *option = NULL;
 char_ptr *a = NULL;
 const char *value = NULL;
 int size = 0;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "option"))
   option = att[1];
  else if (0 == strcmp(*att, "value"))
   value = att[1];
  else
   size += 2;
 }
 if (size)
 {
  a = new char_ptr[size + 1];
  a[size] = NULL;
  size = 0;
  for (int i = 0; atts[i]; ++i)
  {
   if ((i % 2 == 0) && ((0 == strcmp(atts[i], "option")) || (0 == strcmp(atts[i], "value"))))
    ++i;
   else
   {
    a[size] = new char[strlen(atts[i]) + 1];
    strcpy(a[size], atts[i]);
    ++size;
   }
  }
 }
 return new BTCan(option, a, value);
}

void BTScreenSetScreen::draw(BTDisplay &d, ObjectSerializer *obj)
{
 for (int i = 0; i < items.size(); ++i)
 {
  items[i]->draw(d, obj);
 }
}

BTScreenItem *BTScreenSetScreen::findItem(int key)
{
 char utf8Key[5];
 // FIXME: Do real utf-8 conversion
 utf8Key[0] = key;
 utf8Key[1] = 0;
 for (int i = 0; i < items.size(); ++i)
 {
  if (items[i]->getKeys().find(utf8Key) != std::string::npos)
   return items[i];
 }
 return NULL;
}

void BTScreenSetScreen::serialize(ObjectSerializer* s)
{
 s->add("barrier", &items, &BTBarrier::create);
 s->add("line", &items, &BTLine::create);
 s->add("choice", &items, &BTChoice::create);
 s->add("readString", &items, &BTReadString::create);
 s->add("selectRoster", &items, &BTSelectRoster::create);
 s->add("selectRace", &items, &BTSelectRace::create);
 s->add("selectJob", &items, &BTSelectJob::create);
 s->add("selectGoods", &items, &BTSelectGoods::create);
 s->add("selectInventory", &items, &BTSelectInventory::create);
 s->add("selectParty", &items, &BTSelectParty::create);
 s->add("can", &items, &BTCan::create);
}

XMLObject *BTScreenSetScreen::create(const XML_Char *name, const XML_Char **atts)
{
 int number = 0;
 int escapeScreen = 0;
 int timeout = 0;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "number"))
   number = atoi(att[1]);
  else if (0 == strcmp(*att, "escapeScreen"))
  {
   if (0 == strcmp(att[1], "exit"))
    escapeScreen = BTSCREEN_EXIT;
   else
    escapeScreen = atoi(att[1]);
  }
  else if (0 == strcmp(*att, "timeout"))
   timeout = atoi(att[1]);
 }
 return new BTScreenSetScreen(number, escapeScreen, timeout);
}

XMLObject *BTError::create(const XML_Char *name, const XML_Char **atts)
{
 const char *type = NULL;
 int screen(0);
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "type"))
   type = att[1];
  else if (0 == strcmp(*att, "screen"))
   screen = atoi(att[1]);
 }
 return new BTError(type, screen);
}

BTScreenSet::BTScreenSet()
 : picture(-1), label(0), pc(0)
{
 actionList["addToParty"] = &addToParty;
 actionList["buy"] = &buy;
 actionList["create"] = &create;
 actionList["drop"] = &drop;
 actionList["equip"] = &equip;
 actionList["exit"] = &exit;
 actionList["exitAndSave"] = &exitAndSave;
 actionList["give"] = &give;
 actionList["poolGold"] = &poolGold;
 actionList["quit"] = &quit;
 actionList["removeFromParty"] = &removeFromParty;
 actionList["save"] = &save;
 actionList["sell"] = &sell;
 actionList["selectItem"] = &selectItem;
 actionList["selectParty"] = &selectParty;
 actionList["setJob"] = &setJob;
 actionList["setRace"] = &setRace;
 actionList["unequip"] = &unequip;
}

BTScreenSet::~BTScreenSet()
{
 if ((pc) && (0 == pc->name[0]))
  delete pc;
}

int BTScreenSet::getLevel()
{
 return 0;
}

BTPc* BTScreenSet::getPc()
{
 return pc;
}

int BTScreenSet::displayError(BTDisplay &d, const BTSpecialError &e)
{
 d.clearText();
 if (pc)
 {
  add("pc", pc);
  pc->serialize(this);
 }
 BTError *err = NULL;
 for (int i = 0; i < errors.size(); ++i)
 {
  if (0 == strcmp(errors[i]->getType().c_str(), e.error.c_str()))
  {
   err = errors[i];
   break;
  }
 }
 if (err)
  err->draw(d, this);
 else
  d.addText(("Unknown Error: " + e.error).c_str());
 removeLevel();
 d.process(BTDisplay::allKeys);
 if (err)
  return err->getScreen();
 else
  return 0;
}

BTScreenSet::action BTScreenSet::findAction(const std::string &actionName)
{
 std::map<std::string, BTScreenSet::action>::iterator actionItr = actionList.find(actionName);
 if (actionList.end() != actionItr)
  return actionItr->second;
 else
  return NULL;
}

int BTScreenSet::findScreen(int num)
{
 if (num == -1)
  return -1;
 for (int i = 0; i < screen.size(); ++i)
 {
  if (num == screen[i]->getNumber())
   return i;
 }
 return 0;
}

void BTScreenSet::open(const char *filename)
{
 XMLSerializer parser;
 parser.add("picture", &picture);
 parser.add("label", &label);
 parser.add("screen", &screen, &BTScreenSetScreen::create);
 parser.add("error", &errors, &BTError::create);
 parser.parse(filename, true);
}

void BTScreenSet::run(BTDisplay &d, int start /*= 0*/, bool status /*= true*/)
{
 BTParty &party = BTGame::getGame()->getParty();
 std::string itemName;
 char specialKeys[10];
 int previous = 0;
 int where = start;
 d.drawImage(picture);
 d.drawLabel(label);
 while (true)
 {
  d.clearText();
  initScreen(d);
  if (pc)
  {
   add("pc", pc);
   pc->serialize(this);
   if (-1 != pc->combat.item)
   {
    BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
    pc->item[pc->combat.item].serialize(this);
    if (pc->getItem(pc->combat.item) != BTITEM_NONE)
    {
     itemName = itemList[pc->getItem(pc->combat.item)].getName();
     add("itemName", &itemName);
    }
   }
  }
  try
  {
   screen[where]->draw(d, this);
  }
  catch (const BTSpecialError &e)
  {
   removeLevel();
   displayError(d, e);
   where = previous;
   continue;
  }
  previous = where;
  removeLevel();
  if (status)
  {
   for (int i = 0; i < party.size(); ++i)
    specialKeys[i] = i + '1';
  }
  specialKeys[party.size()] = 27;
  specialKeys[party.size() + 1] = 0;
  int key = d.process(specialKeys, screen[where]->getTimeout());
  if (0 == key)
   key = 27;
  endScreen(d);
  BTScreenItem *item = screen[where]->findItem(key);
  if (item)
  {
   int next = 0;

   try
   {
    BTScreenSet::action a = findAction(item->getAction());
    if (a)
     next = (*a)(*this, d, item, key);
   }
   catch (const BTSpecialError &e)
   {
    next = displayError(d, e);
   }
   catch (...)
   {
    d.clearText();
    throw;
   }
   if (0 == next)
    next = item->getScreen(pc);
   where = findScreen(next);
  }
  else if (key == 27)
  {
   int esc = screen[where]->getEscapeScreen();
   if (esc == 0)
    where = start;
   else
    where = findScreen(esc);
  }
  else if ((key >= '1') && (key <= '9'))
  {
   BTGame::getGame()->getStatus().run(d, party[key - '1']);
   d.drawImage(picture);
   d.drawLabel(label);
  }
  else
  {
   where = start;
  }
  if (where == -1)
  {
   try
   {
    exit(*this, d, NULL, 0);
   }
   catch (const BTSpecialError &e)
   {
    removeLevel();
    displayError(d, e);
    where = previous;
    continue;
   }
   catch (...)
   {
    d.clearText();
    throw;
   }
  }
 }
}

void BTScreenSet::setPc(BTPc *c)
{
 if ((pc) && (0 == pc->name[0]))
  delete pc;
 pc = c;
 if (pc)
  pc->combat.item = -1;
}

void BTScreenSet::setPicture(BTDisplay &d, int pic, char *l)
{
 picture = pic;
 d.drawImage(picture);
 label = l;
 d.drawLabel(label);
}

int BTScreenSet::addToParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 BTSelectRoster *select = static_cast<BTSelectRoster*>(item);
 int found;
 for (found = 0; found < party.size(); ++found)
  if (0 == strcmp(roster[select->select]->name, party[found]->name))
  {
   b.setPc(party[found]);
   throw BTSpecialError("inparty");
  }
 if (found >= party.size())
 {
  party.push_back(roster[select->select]);
  d.drawStats();
 }
 select->clear();
 return 0;
}

int BTScreenSet::buy(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectGoods *select = static_cast<BTSelectGoods*>(item);
 if (b.pc->getGold() < itemList[select->select].getPrice())
 {
  throw BTSpecialError("notenoughgold");
 }
 else
 {
  d.drawLast(0, "Done!");
  d.readChar();
  b.pc->takeGold(itemList[select->select].getPrice());
  b.pc->giveItem(select->select, true, itemList[select->select].getTimesUsable());
 }
 return 0;
}

int BTScreenSet::create(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.setPc(new BTPc);
 return 0;
}

int BTScreenSet::drop(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc->takeItemFromIndex(b.pc->combat.item);
 b.pc->combat.item = -1;
 d.drawStats();
 return 0;
}

int BTScreenSet::equip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc->equip(b.pc->combat.item);
 b.pc->combat.item = -1;
 d.drawStats();
 return 0;
}

int BTScreenSet::exit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 bool dead = BTGame::getGame()->getParty().checkDead();
 d.drawStats();
 if (dead)
  throw BTSpecialError("dead");
 else
  throw BTSpecialFlipGoForward();
}

int BTScreenSet::exitAndSave(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTPc::writeXML("roster.xml", roster);
 exit(b, d, item, key);
}

int BTScreenSet::give(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()) && (party[key - '1'] != b.pc))
 {
  BTEquipment &item = b.pc->item[b.pc->combat.item];
  if (party[key - '1']->giveItem(item.id, item.known, item.charges))
  {
   b.pc->takeItemFromIndex(b.pc->combat.item);
   b.pc->combat.item = -1;
   d.drawStats();
  }
  else
  {
   BTPc *current = b.pc;
   b.setPc(party[key - '1']);
   BTSpecialError e("fullinventory");
   int screen = b.displayError(d, e);
   b.removeLevel();
   b.setPc(current);
   return screen;
  }
 }
 return 0;
}

int BTScreenSet::poolGold(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
 {
  if (party[i] != b.pc)
  {
   unsigned int gp = party[i]->getGold();
   party[i]->takeGold(gp - b.pc->giveGold(gp));
  }
 }
}

int BTScreenSet::quit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTPc::writeXML("roster.xml", roster);
 throw BTSpecialQuit();
}

int BTScreenSet::removeFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
 {
  // Add monster to save file
  party.erase(party.begin() + (key - '1'));
  d.drawStats();
 }
 return 0;
}

int BTScreenSet::save(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTReadString *readString = static_cast<BTReadString*>(item);
 if (readString->getResponse().empty())
  return 0;
 int found;
 for (found = 0; found < roster.size(); ++found)
 {
  if (0 == strcmp(roster[found]->name, readString->getResponse().c_str()))
  {
   throw BTSpecialError("exists");
  }
 }
 b.pc->setName(readString->getResponse().c_str());
 roster.push_back(b.pc);
 b.pc = NULL;
 return 0;
}

int BTScreenSet::sell(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 b.pc->giveGold(itemList[b.pc->getItem(select->select)].getPrice() / 2);
 b.pc->takeItemFromIndex(select->select);
 d.drawStats();
 return 0;
}

int BTScreenSet::selectParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9'))
  b.setPc(party[key - '1']);
 return 0;
}

int BTScreenSet::selectItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 b.pc->combat.item = select->select;
 return 0;
}

int BTScreenSet::setJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 BTSelectJob *select = static_cast<BTSelectJob*>(item);
 int count = select->select;
 for (int i = 0; i < job.size(); i++)
 {
  if (job[i]->isAllowed(b.pc, true))
  {
   if (count == 0)
   {
    b.pc->job = i;
    b.pc->picture = job[i]->picture;
    b.pc->hp = b.pc->maxHp = BTDice(1, 14, 14).roll() + ((b.pc->stat[BTSTAT_CN] > 14) ? b.pc->stat[BTSTAT_CN] - 14 : 0);
    b.pc->toHit = job[i]->toHit;
    b.pc->save = job[i]->save + ((b.pc->stat[BTSTAT_LK] > 14) ? b.pc->stat[BTSTAT_LK] - 14 : 0);
    b.pc->gold = BTDice(1, 61, 110).roll();
    if (job[i]->spells)
    {
     b.pc->skill[i] = 1;
     b.pc->sp = b.pc->maxSp = BTDice(1, 8, 9).roll() + ((b.pc->stat[BTSTAT_IQ] > 14) ? b.pc->stat[BTSTAT_IQ] - 14 : 0);
    }
    break;
   }
   --count;
  }
 }
 select->clear();
 return 0;
}

int BTScreenSet::setRace(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
 BTSelectRace *select = static_cast<BTSelectRace*>(item);
 b.pc->race = select->select;
 for (int i = 0; i < BT_STATS; ++i)
  b.pc->stat[i] = race[b.pc->race]->stat[i].roll();
 select->clear();
 return 0;
}

int BTScreenSet::unequip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc->unequip(b.pc->combat.item);
 b.pc->combat.item = -1;
 d.drawStats();
 return 0;
}
