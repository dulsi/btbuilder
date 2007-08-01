/*-------------------------------------------------------------------------*\
  <status.h> -- Status implementation file

  Date      Programmer  Description
  03/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "status.h"
#include "game.h"

class BTStatusGoto
{
 public:
  BTStatusGoto(int pg) : page(pg) {}

  int page;
};

BTStatus::BTStatus(BTDisplay &d, BTPc *c)
: display(d), pc(c), page(0), gotoPage(false)
{
 c->serialize(this);
}

BTStatus::~BTStatus()
{
 display.clearText();
}

void BTStatus::run()
{
 display.drawImage(pc->picture);
 display.drawLabel(pc->name);
 while (true)
 {
  try
  {
   parse("data/status.xml", true);
   break;
  }
  catch (const BTStatusGoto &g)
  {
   page = g.page;
   gotoPage = true;
  }
 }
}

int BTStatus::getLevel()
{
 return 0;
}

void BTStatus::startElement(const XML_Char *name, const XML_Char **atts)
{
 if (0 == strcmp("screen", name))
 {
  for (int i = 0; atts[i]; i += 2)
  {
   if (0 == strcmp("page", atts[i]))
   {
    if (gotoPage)
    {
     if (page == atoi(atts[i + 1]))
      gotoPage = false;
    }
    else
     page = atoi(atts[i + 1]);
   }
  }
  display.clearText();
 }
 else if (gotoPage)
 {
  line.clear();
  return;
 }
 else if (0 == strcmp("line", name))
 {
  line.clear();
 }
 else if (0 == strcmp("inventory", name))
 {
  BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
  std::list<XMLAction*> items;
  char tmp[40];
  int i(0);
  findAll("item", items);
  for (std::list<XMLAction*>::iterator itr(items.begin()); (items.end() != itr); itr++)
  {
   BTEquipment *item = static_cast<BTEquipment*>(reinterpret_cast<XMLObject*>((*itr)->object));
   if (item->id == BTITEM_NONE)
    snprintf(tmp, 40, "%d)", i + 1);
   else
    snprintf(tmp, 40, "%d)%c%s", i + 1, (item->equipped ? '*' : (itemList[item->id].canUse(pc) ? ' ' : '@')), itemList[item->id].getName());
   display.drawText(tmp);
   ++i;
  }
 }
 else if (0 == strcmp("col", name))
 {
  line.push_back("");
 }
 else
 {
  XMLAction *state = find(name, atts);
  if (state)
  {
   if (0 == line.size())
    line.push_back("");
   switch(state->type)
   {
    case XMLTYPE_BOOL:
     if (*(reinterpret_cast<bool*>(state->object)))
      line.back() += "true";
     else
      line.back() += "false";
     break;
    case XMLTYPE_INT:
     if (state->data)
     {
      line.back() += reinterpret_cast<ValueLookup*>(state->data)->getName(*(reinterpret_cast<int*>(state->object)));
     }
     else
     {
      char tmp[40];
      snprintf(tmp, 40, "%d", *(reinterpret_cast<int*>(state->object)));
      line.back() += tmp;
     }
     break;
    case XMLTYPE_UINT:
    {
     char tmp[40];
     snprintf(tmp, 40, "%u", *(reinterpret_cast<unsigned int*>(state->object)));
     line.back() += tmp;
     break;
    }
    case XMLTYPE_STRING:
     line.back() += *(reinterpret_cast<char**>(state->object));
    case XMLTYPE_BITFIELD:
    default:
     break;
   }
  }
 }
}

void BTStatus::endElement(const XML_Char *name)
{
 if (gotoPage)
  return;
 else if (0 == strcmp("screen", name))
 {
  unsigned char key;
  while (true)
  {
   key = display.readChar();
   if (('C' == key) || ('c' == key) || ('D' == key) || ('d' == key))
    break;
   else if (('1' <= key) && ('8' >= key))
   {
    int item = key - '1';
    if (pc->getItem(item) != BTITEM_NONE)
    {
     BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
     display.clearText();
     char tmp[100];
     snprintf(tmp, 100, "%s do you wish to:", pc->name);
     display.drawText(tmp);
     display.drawText("");
     display.drawText("Trade the item");
     display.drawText("Drop the item");
     if (pc->isEquipped(item))
      display.drawText("Unequip the item");
     else if (itemList[pc->getItem(item)].canUse(pc))
      display.drawText("Equip the item");
     display.drawLast("Cc", "(Cancel)", BTDisplay::center);
     while (true)
     {
      key = display.readChar();
      if (('C' == key) || ('c' == key))
       throw BTStatusGoto(page);
      else if ((('E' == key) || ('e' == key)) && (itemList[pc->getItem(item)].canUse(pc)) && (!pc->isEquipped(item)))
      {
       pc->equip(item);
       display.drawStats();
       throw BTStatusGoto(page);
      }
      else if ((('U' == key) || ('u' == key)) && (pc->isEquipped(item)))
      {
       pc->unequip(item);
       display.drawStats();
       throw BTStatusGoto(page);
      }
     }
    }
   }
  }
 }
 else if (0 == strcmp("line", name))
 {
  if (line.size() == 1)
   display.drawText(line.front().c_str());
  if (line.size() == 2)
   display.draw2Column(line.front().c_str(), line.back().c_str());
 }
}

void BTStatus::characterData(const XML_Char *s, int len)
{
 if (0 == line.size())
  line.push_back(std::string(s, len));
 else
  line.back() += std::string(s, len);
}
