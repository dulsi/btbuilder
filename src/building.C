/*-------------------------------------------------------------------------*\
  <building.C> -- Building implementation file

  Date      Programmer  Description
  04/09/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "building.h"
#include "game.h"
#include "pc.h"
#include "status.h"
#include "ikbbuffer.h"

#define BTSCREEN_EXIT -1

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

void BTLine::draw(BTDisplay &d, ObjectSerializer *obj)
{
 std::list<std::string> line;
 for (std::list<std::vector<BTElement*> >::iterator itr = element.begin(); itr != element.end(); ++itr)
  line.push_back(eval(*itr, obj));
 if (line.size() == 1)
  d.addText(line.front().c_str());
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
     case XMLTYPE_BITFIELD:
     default:
      break;
    }
   }
  }
 }
 return final;
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
 d.addChoice(getKeys().c_str(), line.c_str());
}

XMLObject *BTChoice::create(const XML_Char *name, const XML_Char **atts)
{
 BTChoice *obj = new BTChoice();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "keys"))
   obj->setKeys(att[1]);
  else if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
   obj->setScreen(atoi(att[1]));
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
 if (select >= size)
  select = size - 1;
 if (start >= select)
  start = select;
 d.addSelection(list, size, start, select, numbered);
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
 obj->numbered = true;
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
 return size;
}

XMLObject *BTSelectJob::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectJob *obj = new BTSelectJob();
 obj->numbered = true;
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

int BTSelectInventory::buildList(ObjectSerializer *obj)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 XMLAction *act = obj->find("pc", NULL);
 BTPc *pc = static_cast<BTPc*>(reinterpret_cast<XMLObject*>(act->object));
 if (pc->isEquipmentEmpty())
  throw BTSpecialError("noinventory");
 list = new BTDisplay::selectItem[BT_ITEMS];
 int len = 8;
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
  list[i].value = itemList[id].getPrice() / 2;
 }
 return len;
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

void BTBuildingScreen::draw(BTDisplay &d, ObjectSerializer *obj)
{
 for (int i = 0; i < items.size(); ++i)
 {
  items[i]->draw(d, obj);
 }
}

BTScreenItem *BTBuildingScreen::findItem(int key)
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

void BTBuildingScreen::serialize(ObjectSerializer* s)
{
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

XMLObject *BTBuildingScreen::create(const XML_Char *name, const XML_Char **atts)
{
 int number = 0;
 int escapeScreen = 0;
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
 }
 return new BTBuildingScreen(number, escapeScreen);
}

XMLObject *BTError::create(const XML_Char *name, const XML_Char **atts)
{
 const char *type = NULL;
 bool retry(false);
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "type"))
   type = att[1];
  else if (0 == strcmp(*att, "retry"))
   retry = ((strncmp(att[1], "true", 4) == 0) ? true : false);
 }
 return new BTError(type, retry);
}

BTBuilding::BTBuilding(const char *filename)
 : picture(-1), label(0), pc(0)
{
 XMLSerializer parser;
 parser.add("picture", &picture);
 parser.add("label", &label);
 parser.add("screen", &screen, &BTBuildingScreen::create);
 parser.add("error", &errors, &BTError::create);
 parser.parse(filename, true);

 actionList["addToParty"] = &addToParty;
 actionList["buy"] = &buy;
 actionList["create"] = &create;
 actionList["exit"] = &exit;
 actionList["quit"] = &quit;
 actionList["removeFromParty"] = &removeFromParty;
 actionList["save"] = &save;
 actionList["sell"] = &sell;
 actionList["setJob"] = &setJob;
 actionList["setRace"] = &setRace;
}

BTBuilding::~BTBuilding()
{
 if ((pc) && (0 == pc->name[0]))
  delete pc;
}

int BTBuilding::getLevel()
{
 return 0;
}

bool BTBuilding::displayError(BTDisplay &d, const BTSpecialError &e)
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
  return err->getRetry();
 else
  return false;
}

BTBuilding::action BTBuilding::findAction(const std::string &actionName)
{
 std::map<std::string, BTBuilding::action>::iterator actionItr = actionList.find(actionName);
 if (actionList.end() != actionItr)
  return actionItr->second;
 else
  return NULL;
}

int BTBuilding::findScreen(int num)
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

void BTBuilding::run(BTDisplay &d)
{
 BTParty &party = BTGame::getGame()->getParty();
 char specialKeys[10];
 int previous = 0;
 int where = 0;
 d.drawImage(picture);
 d.drawLabel(label);
 while (true)
 {
  d.clearText();
  if (pc)
  {
   add("pc", pc);
   pc->serialize(this);
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
  for (int i = 0; i < party.size(); ++i)
   specialKeys[i] = i + '1';
  specialKeys[party.size()] = 27;
  specialKeys[party.size() + 1] = 0;
  int key = d.process(specialKeys);
  BTScreenItem *item = screen[where]->findItem(key);
  if (item)
  {
   if ((key >= '1') && (key <= '9'))
    setPc(party[key - '1']);
   bool retry(false);
   try
   {
    BTBuilding::action a = findAction(item->getAction());
    if (a)
     (*a)(*this, d, item);
   }
   catch (const BTSpecialError &e)
   {
    retry = displayError(d, e);
   }
   catch (...)
   {
    d.clearText();
    throw;
   }
   if (!retry)
    where = findScreen(item->getScreen(pc));
  }
  else if (key == 27)
  {
   where = findScreen(screen[where]->getEscapeScreen());
  }
  else if ((key >= '1') && (key <= '9'))
  {
   BTStatus s(d, party[key - '1']);
   s.run();
   d.drawImage(picture);
   d.drawLabel(label);
  }
  else
  {
   where = 0;
  }
  if (where == -1)
  {
   try
   {
    exit(*this, d, NULL);
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

void BTBuilding::setPc(BTPc *c)
{
 if ((pc) && (0 == pc->name[0]))
  delete pc;
 pc = c;
}

void BTBuilding::addToParty(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
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
}

void BTBuilding::buy(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
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
  IKeybufferGet();
  b.pc->takeGold(itemList[select->select].getPrice());
  b.pc->giveItem(select->select, true, itemList[select->select].getTimesUsable());
 }
}

void BTBuilding::create(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 b.setPc(new BTPc);
}

void BTBuilding::exit(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 bool dead = BTGame::getGame()->getParty().checkDead();
 d.drawStats();
 if (dead)
  throw BTSpecialError("dead");
 else
  throw BTSpecialFlipGoForward();
}

void BTBuilding::quit(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 throw BTSpecialQuit();
}

void BTBuilding::removeFromParty(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int n = 0; n < party.size(); ++n)
 {
  if (party[n] == b.pc)
  {
   // Add monster to save file
   party.erase(party.begin() + n);
   d.drawStats();
   break;
  }
 }
}

void BTBuilding::save(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTReadString *readString = static_cast<BTReadString*>(item);
 if (readString->getResponse().empty())
  return;
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
}

void BTBuilding::sell(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 b.pc->giveGold(itemList[b.pc->getItem(select->select)].getPrice() / 2);
 b.pc->takeItemFromIndex(select->select);
 d.drawStats();
}

void BTBuilding::setJob(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
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
    b.pc->save = job[i]->save + ((b.pc->stat[BTSTAT_LK] > 14) ? b.pc->stat[BTSTAT_LK] - 14 : 0);
    b.pc->gold = BTDice(1, 61, 110).roll();
    if (job[i]->spells)
    {
     b.pc->spellLvl[i] = 1;
     b.pc->sp = b.pc->maxSp = BTDice(1, 8, 9).roll() + ((b.pc->stat[BTSTAT_IQ] > 14) ? b.pc->stat[BTSTAT_IQ] - 14 : 0);
    }
    break;
   }
   --count;
  }
 }
 select->clear();
}

void BTBuilding::setRace(BTBuilding &b, BTDisplay &d, BTScreenItem *item)
{
 XMLVector<BTRace*> &race = BTGame::getGame()->getRaceList();
 BTSelectRace *select = static_cast<BTSelectRace*>(item);
 b.pc->race = select->select;
 for (int i = 0; i < BT_STATS; ++i)
  b.pc->stat[i] = race[b.pc->race]->stat[i].roll();
 select->clear();
}
