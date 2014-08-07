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

#define BTSCREEN_ADVANCELEVEL 4
#define BTSCREEN_XPNEEDED     3

BTElement::BTElement(const char *name, const char **a)
 : isText(false), text(name), atts(0)
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

std::string BTElement::eval(ObjectSerializer *obj) const
{
 std::string ans;
 if (isText)
  return text;
 else
 {
  XMLAction *state = obj->find(text.c_str(), const_cast<const char**>(atts));
  if (!state)
   state = obj->find(text.c_str(), NULL);
  if (state)
  {
   switch(state->getType())
   {
    case XMLTYPE_BOOL:
    case XMLTYPE_INT:
    case XMLTYPE_UINT:
    case XMLTYPE_INT16:
    case XMLTYPE_UINT16:
    case XMLTYPE_STRING:
    case XMLTYPE_STDSTRING:
     return state->createString();
     break;
    case XMLTYPE_CREATE:
    {
     const char *field = NULL;
     const char *defaultValue = NULL;
     const char **attx = const_cast<const char**>(atts);
     if (attx)
     {
      for (int i = 0; attx[i]; i += 2)
      {
       if (0 == strcmp(attx[i], "field"))
        field = attx[i + 1];
       else if (0 == strcmp(attx[i], "default"))
        defaultValue = attx[i + 1];
      }
      XMLArray *ary = reinterpret_cast<XMLArray*>(state->object);
      bool found(false);
      for (int i = 0; i < ary->size(); ++i)
      {
       XMLLevel *lvl = new XMLLevel;
       lvl->state = state;
       lvl->object = ary->get(i);
       obj->addLevel(lvl);
       found = true;
       for (int i = 0; attx[i]; i += 2)
       {
        if ((0 != strcmp(attx[i], "field")) && (0 != strcmp(attx[i], "default")))
        {
         XMLAction *item = obj->find(attx[i], NULL);
         if ((!item) || (item->createString() != attx[i + 1]))
         {
          found = false;
         }
        }
       }
       if (found)
       {
        XMLAction *item = obj->find(field, NULL);
        if (item)
         ans = item->createString();
        else if (defaultValue)
         ans = defaultValue;
       }
       lvl = obj->removeLevel();
       if (found)
        break;
      }
      if ((!found) && (defaultValue))
       ans = defaultValue;
     }
     break;
    }
    case XMLTYPE_OBJECT:
    {
     // Quick hack to support dice. More complete support needed.
     XMLObject *state_obj = reinterpret_cast<XMLObject*>(state->object);
     BTDice *dice = dynamic_cast<BTDice*>(state_obj);
     if (dice)
     {
      ans = dice->createString();
     }
    }
    case XMLTYPE_BITFIELD:
    default:
     break;
   }
  }
 }
 return ans;
}

std::string BTIf::eval(ObjectSerializer *obj) const
{
 return (cond.check(obj) ? thenClause.eval(obj) : elseClause.eval(obj));
}

void BTIf::serialize(ObjectSerializer* s)
{
 s->add("cond", &cond);
 s->add("then", &thenClause);
 s->add("else", &elseClause);
}

bool BTOperator::check(ObjectSerializer *obj) const
{
 if (type == "eq")
 {
  std::string value = element[0]->eval(obj);
  for (XMLVector<BTSimpleElement*>::const_iterator itr = element.begin() + 1; itr != element.end(); ++itr)
  {
   if (value != (*itr)->eval(obj))
    return false;
  }
  return true;
 }
 else if (type == "ne")
 {
  std::string value = element[0]->eval(obj);
  for (XMLVector<BTSimpleElement*>::const_iterator itr = element.begin() + 1; itr != element.end(); ++itr)
  {
   if (value == (*itr)->eval(obj))
    return false;
  }
  return true;
 }
 else if (type == "gt")
 {
  // Not the most efficient solution since the number is converted to a string and then back to a number.
  long value = atol(element[0]->eval(obj).c_str());
  for (XMLVector<BTSimpleElement*>::const_iterator itr = element.begin() + 1; itr != element.end(); ++itr)
  {
   long value2 = atol((*itr)->eval(obj).c_str());
   if (!(value > value2))
    return false;
   value = value2;
  }
  return true;
 }
}

void BTOperator::serialize(ObjectSerializer* s)
{
}

void BTOperator::elementData(const XML_Char *name, const XML_Char **atts)
{
 element.push_back(new BTElement(name, atts));
}

void BTOperator::characterData(const XML_Char *s, int len)
{
 std::string text(s, len);
 element.push_back(new BTElement(text));
}

bool BTCond::check(ObjectSerializer *obj) const
{
 if (type == "or")
 {
  for (XMLVector<BTCheckTrueFalse*>::const_iterator itr = op.begin(); itr != op.end(); ++itr)
  {
   if ((*itr)->check(obj))
    return true;
  }
  return false;
 }
 else
 {
  bool ans(true);
  for (XMLVector<BTCheckTrueFalse*>::const_iterator itr = op.begin(); itr != op.end(); ++itr)
  {
   ans = ans & (*itr)->check(obj);
  }
  return ans;
 }
}

void BTCond::serialize(ObjectSerializer* s)
{
 s->add("eq", &op, &BTOperator::create);
 s->add("gt", &op, &BTOperator::create);
 s->add("ne", &op, &BTOperator::create);
 s->add("or", &op, &BTCond::create);
}

std::string BTColumn::eval(ObjectSerializer *obj) const
{
 std::string ans;
 for (XMLVector<BTSimpleElement*>::const_iterator itr = element.begin(); itr != element.end(); ++itr)
 {
  ans += (*itr)->eval(obj);
 }
 return ans;
}

void BTColumn::serialize(ObjectSerializer* s)
{
 s->add("if", typeid(BTIf).name(), &element, &BTIf::create);
}

void BTColumn::elementData(const XML_Char *name, const XML_Char **atts)
{
 element.push_back(new BTElement(name, atts));
}

void BTColumn::characterData(const XML_Char *s, int len)
{
 std::string text(s, len);
 element.push_back(new BTElement(text));
}

void BTBarrier::draw(BTDisplay &d, ObjectSerializer *obj)
{
 d.addBarrier("");
}

BTLine::~BTLine()
{
}

void BTLine::setAlignment(std::string a)
{
 align = (BTDisplay::alignment)BTAlignmentLookup::lookup.getIndex(a);
}

void BTLine::draw(BTDisplay &d, ObjectSerializer *obj)
{
 std::list<std::string> line;
 std::string current;
 for (XMLVector<BTSimpleElement*>::const_iterator itr = element.begin(); itr != element.end(); ++itr)
 {
  if (NULL == dynamic_cast<BTColumn*>(*itr))
   current += (*itr)->eval(obj);
  else
   line.push_back((*itr)->eval(obj));
 }
 if (current != "")
  line.push_back(current);
 if (line.size() == 1)
  d.addText(line.front().c_str(), align);
 else if (line.size() == 0)
  d.addText("");
 else
  d.addColumns(line);
}

void BTLine::serialize(ObjectSerializer* s)
{
 s->add("col", typeid(BTColumn).name(), &element, &BTColumn::create);
 s->add("if", typeid(BTIf).name(), &element, &BTIf::create);
}

void BTLine::elementData(const XML_Char *name, const XML_Char **atts)
{
 element.push_back(new BTElement(name, atts));
}

void BTLine::characterData(const XML_Char *s, int len)
{
 std::string text(s, len);
 element.push_back(new BTElement(text));
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
 for (XMLVector<BTSimpleElement*>::iterator itr = element.begin(); itr != element.end(); ++itr)
 {
   line += (*itr)->eval(obj);
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
 for (XMLVector<BTSimpleElement*>::iterator itr = element.begin(); itr != element.end(); ++itr)
 {
   line += (*itr)->eval(obj);
 }
 response = "";
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
 if (list)
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

BTSelectRoster::BTSelectRoster()
 : fullscreen(0)
{
}

int BTSelectRoster::buildList(ObjectSerializer *obj)
{
 XMLVector<BTGroup*> &group = BTGame::getGame()->getGroup();
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 if (0 == roster.size())
  throw BTSpecialError("noroster");
 else if ((fullscreen != 0) && (party.size() >= BT_PARTYSIZE))
  throw BTSpecialError("fullparty");
 int groupSize = group.size();
 list = new BTDisplay::selectItem[groupSize + roster.size()];
 int i;
 for (i = 0; i < group.size(); ++i)
 {
  list[i].first = '*';
  list[i].name = group[i]->name.c_str();
 }
 for (i = 0; i < roster.size(); ++i)
 {
  list[groupSize + i].name = roster[i]->name;
 }
 return groupSize + roster.size();
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
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
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
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
 }
 return obj;
}

BTSelectJob::BTSelectJob()
 : starting(false)
{
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
  if (job[i]->isAllowed(pc, starting))
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
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
  else if (0 == strcmp(*att, "starting"))
   obj->starting = atoi(att[1]);
 }
 return obj;
}

int BTSelectGoods::buildList(ObjectSerializer *obj)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTShop *shopObj = BTGame::getGame()->getShop(shop);
 XMLAction *act = obj->find("pc", NULL);
 BTPc *pc = static_cast<BTPc*>(reinterpret_cast<XMLObject*>(act->object));
 if (pc->isEquipmentFull())
  throw BTSpecialError("fullinventory");
 list = new BTDisplay::selectItem[shopObj->goods.size()];
 for (int i = 0; i < shopObj->goods.size(); )
 {
  if (shopObj->goods[i]->id < itemList.size())
  {
   if (!itemList[shopObj->goods[i]->id].canUse(pc))
    list[i].first = '@';
   list[i].name = itemList[shopObj->goods[i]->id].getName();
   list[i].flags.set(BTSELECTFLAG_SHOWVALUE);
   list[i].value = itemList[shopObj->goods[i]->id].getPrice();
   ++i;
  }
  else
  {
   shopObj->goods.erase(shopObj->goods.begin() + i);
  }
 }
 return shopObj->goods.size();
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
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
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
  if (BTITEM_ARROW == itemList[id].getType())
  {
   char tmp[20];
   snprintf(tmp, 20, "%d", pc->item[i].charges);
   list[i].name = tmp;
   list[i].name += " ";
   list[i].name += itemList[id].getName();
  }
  else
   list[i].name = itemList[id].getName();
  if (value)
  {
   list[i].flags.set(BTSELECTFLAG_SHOWVALUE);
   list[i].value = itemList[id].getPrice() / 2;
  }
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
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
  else if (0 == strcmp(*att, "fullscreen"))
   obj->fullscreen = atoi(att[1]);
  else if (0 == strcmp(*att, "noerror"))
   obj->noerror = atoi(att[1]);
  else if (0 == strcmp(*att, "numbered"))
   obj->numbered = atoi(att[1]);
  else if (0 == strcmp(*att, "value"))
   obj->value = atoi(att[1]);
  else if (0 == strcmp(*att, "shop"))
   obj->shop = atoi(att[1]);
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

void BTSelectParty::checkDisallow(BTPc *pc)
{
 BitField found = pc->status & disallow;
 int num = found.getMaxSet();
 for (int i = 0; i <= num; ++i)
 {
  if (found.isSet(i))
   throw BTSpecialError(BTStatusLookup::lookup.getName(i));
 }
}

int BTSelectParty::getWho()
{
 return who;
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
 int w = 0;
 BitField d;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   act = att[1];
  else if (0 == strcmp(*att, "screen"))
  {
   if (0 == strcmp(att[1], "exit"))
    s = BTSCREEN_EXIT;
   else
    s = atoi(att[1]);
  }
  else if (0 == strcmp(*att, "who"))
  {
   w = atoi(att[1]);
  }
  else if (0 == strcmp(*att, "disallow"))
  {
   int start = 0;
   for (int i = 0; att[1][i]; ++i)
   {
    if (',' == att[1][i])
    {
     int bit = BTStatusLookup::lookup.getIndex(std::string(att[1] + start, i - start));
     if (-1 != bit)
      d.set(bit);
     start = i + 1;
    }
   }
   if (att[1][start])
   {
    int bit = BTStatusLookup::lookup.getIndex(att[1] + start);
    if (-1 != bit)
     d.set(bit);
   }
  }
 }
 return new BTSelectParty(act, s, w, d);
}

BTSelectSong::BTSelectSong()
{
}

int BTSelectSong::buildList(ObjectSerializer *obj)
{
 XMLVector<BTSong*> &songList = BTGame::getGame()->getSongList();
 list = new BTDisplay::selectItem[songList.size()];
 int len = songList.size();
 for (int i = 0; i < len; ++i)
 {
  list[i].name = songList[i]->getName();
 }
 return len;
}

XMLObject *BTSelectSong::create(const XML_Char *name, const XML_Char **atts)
{
 BTSelectSong *obj = new BTSelectSong();
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "action"))
   obj->setAction(att[1]);
  else if (0 == strcmp(*att, "screen"))
  {
   if (0 == strcmp(att[1], "exit"))
    obj->setScreen(BTSCREEN_EXIT);
   else
    obj->setScreen(atoi(att[1]));
  }
  else if (0 == strcmp(*att, "numbered"))
   obj->numbered = atoi(att[1]);
 }
 return obj;
}

BTCan::BTCan(const char *o, char_ptr *a, const char *f, const char *v, const char *d)
: option(o), atts(a), checkValue(false), drawn(false)
{
 if (f)
  field = f;
 if (v)
 {
  value = v;
  checkValue = true;
 }
 if (d)
  defaultValue = d;
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
 if ((drawn) && (items.size() > 0))
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
 if (!state)
  state = obj->find(option.c_str(), NULL);
 if (state)
 {
  switch(state->getType())
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
    if (checkValue)
    {
     if (value != *(reinterpret_cast<char**>(state->object)))
      return;
    }
    else if (0 == strlen(*reinterpret_cast<char**>(state->object)))
     return;
    break;
   case XMLTYPE_STDSTRING:
    if (checkValue)
    {
     if (value != *(reinterpret_cast<std::string*>(state->object)))
      return;
    }
    else if (0 == reinterpret_cast<std::string*>(state->object)->length())
     return;
    break;
   case XMLTYPE_CREATE:
   {
    const char **a = const_cast<const char**>(atts);
    XMLArray *ary = reinterpret_cast<XMLArray*>(state->object);
    bool found(false);
    for (int i = 0; (i < ary->size()) && (!found); ++i)
    {
     XMLLevel *lvl = new XMLLevel;
     lvl->state = state;
     lvl->object = ary->get(i);
     obj->addLevel(lvl);
     found = true;
     for (int i = 0; a[i]; i += 2)
     {
      XMLAction *item = obj->find(atts[i], NULL);
      if ((!item) || (item->createString() != atts[i + 1]))
      {
       found = false;
      }
     }
     lvl = obj->removeLevel();
    }
    if (!found)
     return;
    break;
   }
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
 s->add("barrier", typeid(BTBarrier).name(), &items, &BTBarrier::create);
 s->add("line", typeid(BTLine).name(), &items, &BTLine::create);
 s->add("choice", typeid(BTChoice).name(), &items, &BTChoice::create);
 s->add("readString", typeid(BTReadString).name(), &items, &BTReadString::create);
 s->add("selectRoster", typeid(BTSelectRoster).name(), &items, &BTSelectRoster::create);
 s->add("selectRace", typeid(BTSelectRace).name(), &items, &BTSelectRace::create);
 s->add("selectJob", typeid(BTSelectJob).name(), &items, &BTSelectJob::create);
 s->add("selectGoods", typeid(BTSelectGoods).name(), &items, &BTSelectGoods::create);
 s->add("selectInventory", typeid(BTSelectInventory).name(), &items, &BTSelectInventory::create);
 s->add("selectParty", typeid(BTSelectParty).name(), &items, &BTSelectParty::create);
 s->add("selectSong", typeid(BTSelectSong).name(), &items, &BTSelectSong::create);
}

XMLObject *BTCan::create(const XML_Char *name, const XML_Char **atts)
{
 const char *option = NULL;
 char_ptr *a = NULL;
 const char *field = NULL;
 const char *value = NULL;
 const char *defaultValue = NULL;
 int size = 0;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "option"))
   option = att[1];
  else if (0 == strcmp(*att, "field"))
   field = att[1];
  else if (0 == strcmp(*att, "value"))
   value = att[1];
  else if (0 == strcmp(*att, "defaultValue"))
   defaultValue = att[1];
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
   if ((i % 2 == 0) && ((0 == strcmp(atts[i], "option")) || (0 == strcmp(atts[i], "value")) || (0 == strcmp(atts[i], "field")) || (0 == strcmp(atts[i], "defaultValue"))))
    ++i;
   else
   {
    a[size] = new char[strlen(atts[i]) + 1];
    strcpy(a[size], atts[i]);
    ++size;
   }
  }
 }
 return new BTCan(option, a, field, value, defaultValue);
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
 s->add("barrier", typeid(BTBarrier).name(), &items, &BTBarrier::create);
 s->add("line", typeid(BTLine).name(), &items, &BTLine::create);
 s->add("choice", typeid(BTChoice).name(), &items, &BTChoice::create);
 s->add("readString", typeid(BTReadString).name(), &items, &BTReadString::create);
 s->add("selectRoster", typeid(BTSelectRoster).name(), &items, &BTSelectRoster::create);
 s->add("selectRace", typeid(BTSelectRace).name(), &items, &BTSelectRace::create);
 s->add("selectJob", typeid(BTSelectJob).name(), &items, &BTSelectJob::create);
 s->add("selectGoods", typeid(BTSelectGoods).name(), &items, &BTSelectGoods::create);
 s->add("selectInventory", typeid(BTSelectInventory).name(), &items, &BTSelectInventory::create);
 s->add("selectParty", typeid(BTSelectParty).name(), &items, &BTSelectParty::create);
 s->add("selectSong", typeid(BTSelectSong).name(), &items, &BTSelectSong::create);
 s->add("can", typeid(BTCan).name(), &items, &BTCan::create);
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
  {
   if (0 == strcmp(att[1], "exit"))
    screen = BTSCREEN_EXIT;
   else
    screen = atoi(att[1]);
  }
 }
 return new BTError(type, screen);
}

std::string BTEffect::getAction()
{
 return action;
}

XMLObject *BTEffect::create(const XML_Char *name, const XML_Char **atts)
{
 int type(0);
 const char *action = "";
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "type"))
   type = spellTypeLookup.getIndex(att[1]);
  if (0 == strcmp(*att, "action"))
   action = att[1];
 }
 return new BTEffect(type, action);
}

BTScreenSet::BTScreenSet()
 : picture(-1), label(0), building(false), clearMagic(false), grp(0)
{
 pc[0] = pc[1] = 0;
 actionList["advanceLevel"] = &advanceLevel;
 actionList["addToParty"] = &addToParty;
 actionList["buy"] = &buy;
 actionList["buySkill"] = &buySkill;
 actionList["castNow"] = &castNow;
 actionList["changeJob"] = &changeJob;
 actionList["create"] = &create;
 actionList["drop"] = &drop;
 actionList["dropFromParty"] = &dropFromParty;
 actionList["equip"] = &equip;
 actionList["exit"] = &exit;
 actionList["exitAndSave"] = &exitAndSave;
 actionList["findTraps"] = &findTraps;
 actionList["give"] = &give;
 actionList["identify"] = &identify;
 actionList["moveTo"] = &moveTo;
 actionList["openChest"] = &openChest;
 actionList["poolGold"] = &poolGold;
 actionList["quit"] = &quit;
 actionList["requestSkill"] = &requestSkill;
 actionList["requestJob"] = &requestJob;
 actionList["removeFromParty"] = &removeFromParty;
 actionList["removeRoster"] = &removeRoster;
 actionList["removeTraps"] = &removeTraps;
 actionList["save"] = &save;
 actionList["saveGame"] = &saveGame;
 actionList["saveParty"] = &saveParty;
 actionList["sell"] = &sell;
 actionList["selectBard"] = &selectBard;
 actionList["selectItem"] = &selectItem;
 actionList["selectMage"] = &selectMage;
 actionList["selectParty"] = &selectParty;
 actionList["selectRoster"] = &selectRoster;
 actionList["setJob"] = &setJob;
 actionList["setGender"] = &setGender;
 actionList["setRace"] = &setRace;
 actionList["singNow"] = &singNow;
 actionList["tradeGold"] = &tradeGold;
 actionList["unequip"] = &unequip;
 actionList["useNow"] = &useNow;
 actionList["useOn"] = &useOn;
}

BTScreenSet::~BTScreenSet()
{
 if ((pc[0]) && (0 == pc[0]->name[0]))
  delete pc[0];
 if (label)
  delete [] label;
}

BTPc* BTScreenSet::getPc()
{
 return pc[0];
}

void BTScreenSet::checkEffects(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 for (int i = 0; i < effects.size(); ++i)
 {
  if ((!effects[i]->isProcessed()) && (game->hasEffectOfType(effects[i]->getType())))
  {
   effects[i]->setProcessed();
   effects[i]->draw(d, this);
   d.process(BTDisplay::allKeys);
   try
   {
    BTScreenSet::action a = findAction(effects[i]->getAction());
    if (a)
     (*a)(*this, d, effects[i], 13);
   }
   catch (const BTSpecialError &e)
   {
    displayError(d, e);
   }
   d.clearText();
  }
 }
}

int BTScreenSet::displayError(BTDisplay &d, const BTSpecialError &e)
{
 d.clearText();
 if (pc[0])
 {
  add("pc", pc[0]);
  pc[0]->serialize(this);
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
 parser.add("building", &building);
 parser.add("clearMagic", &clearMagic);
 parser.add("screen", &screen, &BTScreenSetScreen::create);
 parser.add("error", &errors, &BTError::create);
 parser.add("effect", &effects, &BTEffect::create);
 parser.parse(filename, true);
}

void BTScreenSet::run(BTDisplay &d, int start /*= 0*/, bool status /*= true*/)
{
 BTGame *game = BTGame::getGame();
 if (clearMagic)
 {
  game->clearEffects(d);
  game->checkExpiration(d, NULL);
  d.drawIcons();
  game->clearTimedSpecial();
  game->resetTime();
 }
 BTParty &party = game->getParty();
 std::string itemName;
 char specialKeys[10];
 int previous = 0;
 int where = start;
 int partySize = party.size();
 if (picture != -1)
  d.drawImage(picture);
 if (label)
  d.drawLabel(label);
 try
 {
  bool init = true;
  while (true)
  {
   d.clearText();
   if (init)
   {
    partySize = party.size();
    initScreen(d);
    if (grp)
    {
     add("party", grp);
     grp->serialize(this);
    }
    if (pc[0])
    {
     add("pc", pc[0]);
     add("partySize", &partySize);
     pc[0]->serialize(this);
     switch (pc[0]->combat.type)
     {
      case BTPc::BTPcAction::item:
      {
       BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
       pc[0]->item[pc[0]->combat.object].serialize(this);
       if (pc[0]->getItem(pc[0]->combat.object) != BTITEM_NONE)
       {
        itemName = itemList[pc[0]->getItem(pc[0]->combat.object)].getName();
        add("itemName", &itemName);
       }
       setNamespace("item");
       itemList[pc[0]->getItem(pc[0]->combat.object)].serialize(this);
       setNamespace("");
       break;
      }
      case BTPc::BTPcAction::spell:
       break;
      default:
       break;
     }
    }
   }
   checkEffects(d);
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
   init = true;
   if (status)
   {
    for (int i = 0; i < party.size(); ++i)
     specialKeys[i] = i + '1';
   }
   specialKeys[party.size()] = 27;
   specialKeys[party.size() + 1] = 0;
   int key = d.process(specialKeys, game->getDelay(), screen[where]->getTimeout());
   if (0 == key)
    key = 27;
   endScreen(d);
   BTScreenItem *item = screen[where]->findItem(key);
   if (item)
   {
    int next = 0;

   removeLevel();
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
    if (BTSCREEN_ESCAPE == next)
     next = screen[where]->getEscapeScreen();
    if (0 == next)
     next = item->getScreen(pc[0]);
    where = findScreen(next);
   }
   else if (key == 27)
   {
    removeLevel();
    int esc = screen[where]->getEscapeScreen();
    if (esc == 0)
     where = start;
    else
     where = findScreen(esc);
   }
   else if ((key >= '1') && (key <= '9'))
   {
    init = false;
    BTGame::getGame()->getStatus().run(d, party[key - '1']);
    d.drawImage(picture);
    d.drawLabel(label);
   }
   else
   {
    removeLevel();
    where = start;
   }
   if (where == BTSCREEN_EXIT)
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
   }
  }
 }
 catch (const BTSpecialStop &e)
 {
  if (building)
  {
   d.clearText();
   throw BTSpecialFlipGoForward();
  }
  else
   d.clearElements();
 }
}

void BTScreenSet::setEffect(int type)
{
 for (int i = 0; i < effects.size(); ++i)
 {
  if (effects[i]->getType() == type)
  {
   effects[i]->setProcessed();
  }
 }
}

void BTScreenSet::setGroup(BTGroup *g)
{
 if ((pc[0]) && (0 == pc[0]->name[0]))
  delete pc[0];
 pc[0] = 0;
 grp = g;
}

void BTScreenSet::setPc(BTPc *c, int who /*= 0*/)
{
 if ((pc[who]) && (0 == pc[who]->name[0]))
  delete pc[who];
 pc[who] = c;
 if (pc[who])
 {
  pc[who]->combat.object = -1;
  pc[who]->combat.type = BTPc::BTPcAction::none;
 }
 grp = 0;
}

void BTScreenSet::setPicture(BTDisplay &d, int pic, const char *l)
{
 picture = pic;
 d.drawImage(picture);
 if (label)
  delete [] label;
 if (l)
 {
  int len = strlen(l);
  label = new char[len + 1];
  strcpy(label, l);
  d.drawLabel(label);
 }
 else
  label = 0;
}

int BTScreenSet::advanceLevel(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 bool wasDrained = (b.pc[0]->level < b.pc[0]->maxLevel);
 if (b.pc[0]->advanceLevel())
 {
  int stat = (wasDrained ? -1 : b.pc[0]->incrementStat());
  d.drawStats();
  if (stat != -1)
   b.add("increaseStat", &statAbbrev[stat]);
  return BTSCREEN_ADVANCELEVEL;
 }
 else
 {
  b.add("xpneeded", new unsigned int(b.pc[0]->getXPNeeded()), NULL, true);
  return BTSCREEN_XPNEEDED;
 }
}

int BTScreenSet::addToParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTGroup*> &group = BTGame::getGame()->getGroup();
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 BTSelectRoster *select = static_cast<BTSelectRoster*>(item);
 if (select->select < group.size())
 {
  for (int i = 0; i < group[select->select]->member.size(); ++i)
  {
   int found;
   for (found = 0; found < party.size(); ++found)
    if (0 == strcmp(group[select->select]->member[i].c_str(), party[found]->name))
    {
     break;
    }
   if (found >= party.size())
   {
    for (int k = 0; k < roster.size(); ++k)
    {
     if (0 == strcmp(roster[k]->name, group[select->select]->member[i].c_str()))
     {
      party.push_back(roster[k]);
      d.drawStats();
      if (party.size() >= BT_PARTYSIZE)
       return 0;
     }
    }
   }
  }
 }
 else
 {
  int roosterSelect = select->select - group.size();
  int found;
  for (found = 0; found < party.size(); ++found)
   if (0 == strcmp(roster[roosterSelect]->name, party[found]->name))
   {
    b.setPc(party[found]);
    throw BTSpecialError("inparty");
   }
  if (found >= party.size())
  {
   party.push_back(roster[roosterSelect]);
   d.drawStats();
  }
  select->clear();
 }
 return 0;
}

int BTScreenSet::buy(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectGoods *select = static_cast<BTSelectGoods*>(item);
 BTShop *shop = BTGame::getGame()->getShop(select->shop);
 if (b.pc[0]->getGold() < itemList[shop->goods[select->select]->id].getPrice())
 {
  throw BTSpecialError("notenoughgold");
 }
 else
 {
  d.drawLast(0, "Done!");
  d.readChar();
  b.pc[0]->takeGold(itemList[shop->goods[select->select]->id].getPrice());
  b.pc[0]->giveItem(shop->goods[select->select]->id, true, itemList[shop->goods[select->select]->id].getTimesUsable());
  shop->removeItem(shop->goods[select->select]->id);
 }
 return 0;
}

int BTScreenSet::buySkill(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 BTSkillList &skill = BTGame::getGame()->getSkillList();
 bool bFound(false);
 for (int sk = 0; sk < job[b.pc[0]->job]->skill.size(); ++sk)
 {
  BTJobSkillPurchase *purchase = job[b.pc[0]->job]->skill[sk]->findNextPurchase(b.pc[0]->getSkill(job[b.pc[0]->job]->skill[sk]->skill));
  if ((NULL != purchase) && (b.pc[0]->level >= purchase->minimumLevel))
  {
   if (b.pc[0]->getGold() < purchase->cost)
   {
    throw BTSpecialError("notenoughgold");
   }
   else
   {
    b.pc[0]->takeGold(purchase->cost);
    b.pc[0]->setSkill(job[b.pc[0]->job]->skill[sk]->skill, purchase->value, purchase->value);
   }
   return 0;
  }
  else if (NULL != purchase)
   bFound = true;
 }
 if (bFound)
  throw BTSpecialError("notminimumlevel");
 else
  throw BTSpecialError("noskill");
}

int BTScreenSet::castNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
 BTReadString *readString = static_cast<BTReadString*>(item);
 std::string spellCode = readString->getResponse();
 for (int i = 0; i < spellList.size(); ++i)
 {
  if (0 == strcasecmp(spellCode.c_str(), spellList[i].getCode()))
  {
   if (b.getPc()->getSkill(spellList[i].getCaster()) >= spellList[i].getLevel())
   {
    if (b.getPc()->sp < spellList[i].getSp())
     throw BTSpecialError("nosp");
    int pcNumber = 0;
    for (int k = 0; k < party.size(); ++k)
    {
     if (party[k] == b.getPc())
     {
      pcNumber = k;
      break;
     }
    }
    switch (spellList[i].getArea())
    {
     case BTAREAEFFECT_FOE:
      b.getPc()->combat.action = BTPc::BTPcAction::cast;
      b.getPc()->combat.object = i;
      b.getPc()->combat.type = BTPc::BTPcAction::spell;
      return 0;
     case BTAREAEFFECT_GROUP:
      d.clearText();
      b.pc[0]->sp -= spellList[i].getSp();
      d.drawStats();
      spellList[i].cast(d, b.pc[0]->name, BTTARGET_PARTY, pcNumber, true, NULL, b.pc[0]->level, 0, BTTARGET_PARTY, BTTARGET_INDIVIDUAL);
      return BTSCREEN_ESCAPE;
     case BTAREAEFFECT_NONE:
      d.clearText();
      b.pc[0]->sp -= spellList[i].getSp();
      d.drawStats();
      spellList[i].cast(d, b.pc[0]->name, BTTARGET_PARTY, pcNumber, true, NULL, b.pc[0]->level, 0, 0, BTTARGET_INDIVIDUAL);
      return BTSCREEN_ESCAPE;
     case BTAREAEFFECT_ALL:
      throw BTSpecialError("nocombat");
     default:
      return BTSCREEN_ESCAPE;
    }
   }
  }
 }
 throw BTSpecialError("nospell");
}

int BTScreenSet::changeJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 BTSelectJob *select = static_cast<BTSelectJob*>(item);
 int count = select->select;
 for (int i = 0; i < job.size(); i++)
 {
  if (job[i]->isAllowed(b.pc[0], select->starting))
  {
   if (count == 0)
   {
    b.pc[0]->changeJob(i);
    d.drawStats();
    break;
   }
   --count;
  }
 }
 select->clear();
 return 0;
}

int BTScreenSet::create(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.setPc(new BTPc);
 return 0;
}

int BTScreenSet::drop(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc[0]->takeItemFromIndex(b.pc[0]->combat.object);
 b.pc[0]->combat.object = -1;
 b.pc[0]->combat.type = BTPc::BTPcAction::none;
 d.drawStats();
 return 0;
}

int BTScreenSet::dropFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
 {
  if ((party[key - '1']->job != BTJOB_MONSTER) && (party[key - '1']->job != BTJOB_ILLUSION))
   throw BTSpecialError("nodrop");
  BTGame::getGame()->movedPlayer(d, key - '1', BTPARTY_REMOVE);
  party.remove(key - '1', d);
  d.drawStats();
 }
 return 0;
}

int BTScreenSet::equip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc[0]->equip(b.pc[0]->combat.object);
 b.pc[0]->combat.object = -1;
 b.pc[0]->combat.type = BTPc::BTPcAction::none;
 d.drawStats();
 return 0;
}

int BTScreenSet::exit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 bool dead = BTGame::getGame()->getParty().checkDead(d);
 if (item)
  d.clearText();
 d.drawStats();
 if (dead)
 {
  // The guild needs to display a message if you have a dead party but
  // everything else wants out of the BTScreenSet handling.
  if (b.building)
   throw BTSpecialError("dead");
  else
   throw BTSpecialDead();
 }
 else
  throw BTSpecialStop();
}

int BTScreenSet::exitAndSave(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame::getGame()->save();
 exit(b, d, item, key);
}

int BTScreenSet::findTraps(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 BTChest &chest = BTGame::getGame()->getChest();
 BTSkillList &skillList = BTGame::getGame()->getSkillList();
 if (chest.isSearched())
 {
  throw BTSpecialError("alreadysearched");
 }
 chest.setSearched();
 for (int i = 0; i < skillList.size(); ++i)
 {
  if (skillList[i]->special == BTSKILLSPECIAL_DISARM)
  {
   for (int k = 0; k < party.size(); ++k)
   {
    if (party[k]->useSkill(i))
    {
     b.setPc(party[k]);
     chest.removeTrap();
     throw BTSpecialError("findtrap");
    }
   }
  }
 }
 throw BTSpecialError("notraps");
}

int BTScreenSet::give(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()) && (party[key - '1'] != b.pc[0]))
 {
  BTEquipment &item = b.pc[0]->item[b.pc[0]->combat.object];
  if (party[key - '1']->giveItem(item.id, item.known, item.charges))
  {
   b.pc[0]->takeItemFromIndex(b.pc[0]->combat.object);
   b.pc[0]->combat.object = -1;
   b.pc[0]->combat.type = BTPc::BTPcAction::none;
   d.drawStats();
  }
  else
  {
   BTPc *current = b.pc[0];
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

int BTScreenSet::identify(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 if (b.pc[0]->getGold() < (itemList[b.pc[0]->getItem(select->select)].getPrice() / 2))
 {
  throw BTSpecialError("notenoughgold");
 }
 b.pc[0]->takeGold(itemList[b.pc[0]->getItem(select->select)].getPrice() / 2);
 b.pc[0]->combat.object = select->select;
 b.pc[0]->combat.type = BTPc::BTPcAction::item;
 return 0;
}

int BTScreenSet::moveTo(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()) && (party[key - '1'] != b.pc[0]))
 {
  for (int i = 0; i < party.size(); ++i)
  {
   if (party[i] == b.pc[0])
   {
    if (i != key - '1')
    {
     party.moveTo(i, key - '1', d);
    }
    break;
   }
  }
 } 
 d.clearText();
 return 0;
}

int BTScreenSet::openChest(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 d.clearText();
 BTGame::getGame()->getChest().open(d);
 return 0;
}

int BTScreenSet::poolGold(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 for (int i = 0; i < party.size(); ++i)
 {
  if (party[i] != b.pc[0])
  {
   unsigned int gp = party[i]->getGold();
   party[i]->takeGold(gp - b.pc[0]->giveGold(gp));
  }
 }
 return 0;
}

int BTScreenSet::quit(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame::getGame()->save();
 throw BTSpecialQuit();
}

int BTScreenSet::requestSkill(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 BTSkillList &skill = BTGame::getGame()->getSkillList();
 bool bFound(false);
 for (int sk = 0; sk < job[b.pc[0]->job]->skill.size(); ++sk)
 {
  BTJobSkillPurchase *purchase = job[b.pc[0]->job]->skill[sk]->findNextPurchase(b.pc[0]->getSkill(job[b.pc[0]->job]->skill[sk]->skill));
  if ((purchase) && (b.pc[0]->level >= purchase->minimumLevel))
  {
   b.add("num", &purchase->value);
   b.add("cost", &purchase->cost);
   return 0;
  }
  else
   bFound = true;
 }
 if (bFound)
  throw BTSpecialError("notminimumlevel");
 else
  throw BTSpecialError("noskill");
}

int BTScreenSet::requestJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 bool bFound(false);
 for (int i = 0; i < job.size(); i++)
 {
  if ((i != b.pc[0]->job)  && (job[i]->isAllowed(b.pc[0], false)))
  {
   bFound = true;
   break;
  }
 }
 if (!bFound)
  throw BTSpecialError("nojob");
 return 0;
}

int BTScreenSet::removeFromParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
 {
  BTGame::getGame()->movedPlayer(d, key - '1', BTPARTY_REMOVE);
  // Add monster to save file
  bool found = false;
  BTPc *pc = party[key - '1'];
  for (XMLVector<BTPc*>::iterator itr = roster.begin(); itr != roster.end(); ++itr)
  {
   if (pc == (*itr))
   {
    found = true;
    break;
   }
  }
  if (!found)
  {
   roster.push_back(pc);
  }
  party.remove(key - '1', d);
  d.drawStats();
 }
 return 0;
}

int BTScreenSet::removeRoster(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTGroup*> &group = BTGame::getGame()->getGroup();
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTParty &party = BTGame::getGame()->getParty();
 for (XMLVector<BTGroup*>::iterator itr = group.begin(); itr != group.end(); ++itr)
 {
  if ((*itr) == b.grp)
  {
   group.erase(itr);
   b.setGroup(NULL);
   return 0;
  }
 }
 for (XMLVector<BTPc*>::iterator itr = roster.begin(); itr != roster.end(); ++itr)
 {
  if ((*itr) == b.pc[0])
  {
   roster.erase(itr, false);
   bool found = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i] == b.pc[0])
     found = true;
   if (!found)
   {
    b.pc[0]->setName("");
   }
   b.setPc(NULL);
   break;
  }
 }
 return 0;
}

int BTScreenSet::removeTraps(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTChest &chest = BTGame::getGame()->getChest();
 chest.removeTrap();
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
 b.pc[0]->setName(readString->getResponse().c_str());
 roster.push_back(b.pc[0]);
 b.pc[0] = NULL;
 return 0;
}

int BTScreenSet::saveGame(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame::getGame()->writeSaveXML("savegame.xml");
 return 0;
}

int BTScreenSet::saveParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTGroup*> &group = BTGame::getGame()->getGroup();
 BTParty &party = BTGame::getGame()->getParty();
 BTReadString *readString = static_cast<BTReadString*>(item);
 if (readString->getResponse().empty())
  return 0;
 int found;
 for (found = 0; found < group.size(); ++found)
 {
  if (group[found]->name == readString->getResponse())
  {
   throw BTSpecialError("existsparty");
  }
 }
 BTGroup *grp = new BTGroup;
 grp->name = readString->getResponse();
 for (int i = 0; i < party.size(); ++i)
  grp->member.push_back(party[i]->name);
 group.push_back(grp);
 return 0;
}

int BTScreenSet::sell(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 if (select->shop > 0)
 {
  BTShop *shop = BTGame::getGame()->getShop(select->shop);
  shop->addItem(b.pc[0]->getItem(select->select));
 }
 b.pc[0]->giveGold(itemList[b.pc[0]->getItem(select->select)].getPrice() / 2);
 b.pc[0]->takeItemFromIndex(select->select);
 d.drawStats();
 return 0;
}

int BTScreenSet::selectBard(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 XMLVector<BTSkill*> &skill = BTGame::getGame()->getSkillList();
 if ((key >= '1') && (key <= '9'))
 {
  for (int i = 0; i < skill.size(); ++i)
  {
   if ((skill[i]->special == BTSKILLSPECIAL_SONG) && (party[key - '1']->getSkill(i) > 0))
   {
    BTSelectParty *select = static_cast<BTSelectParty*>(item);
    b.setPc(party[key - '1'], select->getWho());
    select->checkDisallow(b.pc[select->getWho()]);
    return 0;
   }
  }
  throw BTSpecialError("nobard");
 }
 return 0;
}

int BTScreenSet::selectMage(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9'))
 {
  if (party[key - '1']->maxSp > 0)
  {
   BTSelectParty *select = static_cast<BTSelectParty*>(item);
   b.setPc(party[key - '1'], select->getWho());
   select->checkDisallow(b.pc[select->getWho()]);
  }
  else
   throw BTSpecialError("nocaster");
 }
 return 0;
}

int BTScreenSet::selectParty(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 if ((key >= '1') && (key <= '9'))
 {
  BTSelectParty *select = static_cast<BTSelectParty*>(item);
  b.setPc(party[key - '1'], select->getWho());
  select->checkDisallow(b.pc[select->getWho()]);
 }
 return 0;
}

int BTScreenSet::selectItem(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 b.pc[0]->combat.object = select->select;
 b.pc[0]->combat.type = BTPc::BTPcAction::item;
 return 0;
}

int BTScreenSet::selectRoster(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTGroup*> &group = BTGame::getGame()->getGroup();
 XMLVector<BTPc*> &roster = BTGame::getGame()->getRoster();
 BTSelectRoster *select = static_cast<BTSelectRoster*>(item);
 if (select->select < group.size())
  b.setGroup(group[select->select]);
 else
  b.setPc(roster[select->select - group.size()]);
 return 0;
}

int BTScreenSet::setGender(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 if ((key == 'f') || (key == 'F'))
  b.pc[0]->gender = BTGENDER_FEMALE;
 else if ((key == 'm') || (key == 'M'))
  b.pc[0]->gender = BTGENDER_MALE;
 return 0;
}

int BTScreenSet::setJob(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 XMLVector<BTJob*> &job = BTGame::getGame()->getJobList();
 BTSelectJob *select = static_cast<BTSelectJob*>(item);
 int count = select->select;
 for (int i = 0; i < job.size(); i++)
 {
  if (job[i]->isAllowed(b.pc[0], true))
  {
   if (count == 0)
   {
    b.pc[0]->job = i;
    if (b.pc[0]->gender == BTGENDER_FEMALE)
     b.pc[0]->picture = job[i]->femalePicture;
    else
     b.pc[0]->picture = job[i]->malePicture;
    b.pc[0]->hp = b.pc[0]->maxHp = BTDice(1, 14, 14).roll() + ((b.pc[0]->stat[BTSTAT_CN] > 14) ? b.pc[0]->stat[BTSTAT_CN] - 14 : 0);
    b.pc[0]->toHit = job[i]->toHit;
    b.pc[0]->save = job[i]->save + ((b.pc[0]->stat[BTSTAT_LK] > 14) ? b.pc[0]->stat[BTSTAT_LK] - 14 : 0);
    b.pc[0]->ac = job[i]->ac + ((b.pc[0]->stat[BTSTAT_DX] > 14) ? b.pc[0]->stat[BTSTAT_DX] - 14 : 0);
    for (int k = 0; k < job[i]->skill.size(); ++k)
    {
     int value = job[i]->skill[k]->value;
     if ((job[i]->skill[k]->modifier >= 0) && (b.pc[0]->stat[job[i]->skill[k]->modifier] > 14))
      value += b.pc[0]->stat[job[i]->skill[k]->modifier] - 14;
     b.pc[0]->setSkill(job[i]->skill[k]->skill, value, value);
    }
    b.pc[0]->gold = BTDice(1, 61, 110).roll();
    if (job[i]->spells)
    {
     b.pc[0]->sp = b.pc[0]->maxSp = BTDice(1, 8, 9).roll() + ((b.pc[0]->stat[BTSTAT_IQ] > 14) ? b.pc[0]->stat[BTSTAT_IQ] - 14 : 0);
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
 b.pc[0]->race = select->select;
 for (int i = 0; i < BT_STATS; ++i)
  b.pc[0]->stat[i] = b.pc[0]->statMax[i] = race[b.pc[0]->race]->stat[i].roll();
 select->clear();
 return 0;
}

int BTScreenSet::singNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTGame *game = BTGame::getGame();
 XMLVector<BTSong*> &songList = game->getSongList();
 XMLVector<BTSkill*> &skill = game->getSkillList();
 BTFactory<BTItem> &itemList = game->getItemList();
 BTSelectSong *select = static_cast<BTSelectSong*>(item);
 for (int i = 0; i < skill.size(); ++i)
 {
  if ((skill[i]->special == BTSKILLSPECIAL_SONG) && (b.pc[0]->hasSkillUse(i)))
  {
   bool instrument(false);
   for (int k = 0; k < BT_ITEMS; ++k)
   {
    if ((b.pc[0]->item[k].equipped == BTITEM_EQUIPPED) && (itemList[b.pc[0]->item[k].id].getType() == BTITEM_INSTRUMENT))
    {
     instrument = true;
     break;
    }
   }
   if (!instrument)
    throw BTSpecialError("noinstrument");
   if (game->getFlags().isSet(BTSPECIALFLAG_SILENCE))
    throw BTSpecialError("silence");
   d.clearText();
   b.pc[0]->giveSkillUse(i, -1);
   game->clearEffectsBySource(d, true);
   game->checkExpiration(d, NULL);
   d.drawIcons();
   songList[select->select]->play(d, b.pc[0], NULL);
   d.drawIcons();
   d.drawStats();
   return BTSCREEN_ESCAPE;
  }
 }
 throw BTSpecialError("novoice");
}

int BTScreenSet::tradeGold(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTReadString *readString = static_cast<BTReadString*>(item);
 if (readString->getResponse().empty())
  return 0;
 int gold = atoi(readString->getResponse().c_str());
 if (b.pc[0]->getGold() < gold)
  throw BTSpecialError("notenoughgold");
 b.pc[0]->takeGold(gold);
 b.pc[1]->giveGold(gold);
 return 0;
}

int BTScreenSet::unequip(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 b.pc[0]->unequip(b.pc[0]->combat.object);
 b.pc[0]->combat.object = -1;
 b.pc[0]->combat.type = BTPc::BTPcAction::none;
 d.drawStats();
 return 0;
}

int BTScreenSet::useNow(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 BTParty &party = BTGame::getGame()->getParty();
 BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
 BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
 BTSelectInventory *select = static_cast<BTSelectInventory*>(item);
 if ((select->select == -1) || (b.pc[0]->item[select->select].id == BTITEM_NONE))
 {
  d.clearText();
  return BTSCREEN_ESCAPE;
 }
 if (b.pc[0]->item[select->select].charges == 0)
  throw BTSpecialError("notusable");
 else if (BTITEM_ARROW == itemList[b.pc[0]->item[select->select].id].getType())
 {
  if (BTITEM_CANNOTEQUIP == b.pc[0]->item[select->select].equipped)
   throw BTSpecialError("notbyyou");
  // Determine if you have a bow equipped.
  bool found = false;
  for (int i = 0; i < BT_ITEMS; ++i)
  {
   if (BTITEM_NONE == b.pc[0]->item[i].id)
    break;
   if ((BTITEM_EQUIPPED == b.pc[0]->item[i].equipped) && (BTITEM_BOW == itemList[b.pc[0]->item[i].id].getType()))
   {
    found = true;
    break;
   }
  }
  if (!found)
   throw BTSpecialError("nobow");
 }
 else if (BTITEM_THROWNWEAPON == itemList[b.pc[0]->item[select->select].id].getType())
 {
  // Allow even if not equipped
  if (BTITEM_CANNOTEQUIP == b.pc[0]->item[select->select].equipped)
   throw BTSpecialError("notbyyou");
 }
 else if (BTITEM_EQUIPPED != b.pc[0]->item[select->select].equipped)
  throw BTSpecialError("notequipped");
 else if (BTITEM_BOW == itemList[b.pc[0]->item[select->select].id].getType())
  throw BTSpecialError("notarrow");
 else
 {
  int spellCast = itemList[b.pc[0]->item[select->select].id].getSpellCast();
  if (spellCast == BTITEMCAST_NONE)
   throw BTSpecialError("notusable");
  int pcNumber = 0;
  for (int k = 0; k < party.size(); ++k)
  {
   if (party[k] == b.getPc())
   {
    pcNumber = k;
    break;
   }
  }
  switch (spellList[spellCast].getArea())
  {
   case BTAREAEFFECT_FOE:
    // Fall through to code for bows and thrown weapons
    break;
   case BTAREAEFFECT_GROUP:
    d.clearText();
    b.pc[0]->takeItemCharge(select->select);
    d.drawStats();
    spellList[spellCast].cast(d, b.pc[0]->name, BTTARGET_PARTY, pcNumber, true, NULL, b.pc[0]->level, 0, BTTARGET_PARTY, BTTARGET_INDIVIDUAL);
    return BTSCREEN_ESCAPE;
   case BTAREAEFFECT_NONE:
    d.clearText();
    b.pc[0]->takeItemCharge(select->select);
    d.drawStats();
    spellList[spellCast].cast(d, b.pc[0]->name, BTTARGET_PARTY, pcNumber, true, NULL, b.pc[0]->level, 0, 0, BTTARGET_INDIVIDUAL);
    return BTSCREEN_ESCAPE;
   case BTAREAEFFECT_ALL:
    throw BTSpecialError("nocombat");
   default:
    return BTSCREEN_ESCAPE;
  }
 }
 b.getPc()->combat.action = BTPc::BTPcAction::useItem;
 b.pc[0]->combat.object = select->select;
 b.pc[0]->combat.type = BTPc::BTPcAction::item;
 return 0;
}

int BTScreenSet::useOn(BTScreenSet &b, BTDisplay &d, BTScreenItem *item, int key)
{
 if (b.pc[0]->combat.action == BTPc::BTPcAction::cast)
 {
  d.clearText();
  BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
  b.pc[0]->sp -= spellList[b.pc[0]->combat.object].getSp();
  d.drawStats();
  spellList[b.pc[0]->combat.object].cast(d, b.pc[0]->name, BTTARGET_NONE, BTTARGET_INDIVIDUAL, true, NULL, b.pc[0]->level, 0, BTTARGET_PARTY, key - '1');
  return -1;
 }
 else if (b.pc[0]->combat.action == BTPc::BTPcAction::useItem)
 {
  d.clearText();
  BTParty &party = BTGame::getGame()->getParty();
  BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
  if ((BTITEM_ARROW == itemList[b.pc[0]->item[b.pc[0]->combat.object].id].getType()) || (BTITEM_THROWNWEAPON == itemList[b.pc[0]->item[b.pc[0]->combat.object].id].getType()))
  {
   if ((key >= '1') && (key <= '9') && (key - '1' < party.size()))
   {
    int numAttacks = 1;
    int activeNum = 1;
    std::string text = b.pc[0]->attack(party[key - '1'], b.pc[0]->item[b.pc[0]->combat.object].id, numAttacks, activeNum);
    d.drawMessage(text.c_str(), BTGame::getGame()->getDelay());
   }
  }
  else
  {
   BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
   int spellCast = itemList[b.pc[0]->item[b.pc[0]->combat.object].id].getSpellCast();
   d.drawStats();
   spellList[spellCast].cast(d, b.pc[0]->name, BTTARGET_NONE, BTTARGET_INDIVIDUAL, true, NULL, b.pc[0]->level, 0, BTTARGET_PARTY, key - '1');
  }
  b.pc[0]->takeItemCharge(b.pc[0]->combat.object);
  return -1;
 }
 return 0;
}

