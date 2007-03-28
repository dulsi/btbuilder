/*-------------------------------------------------------------------------*\
  <status.h> -- Status implementation file

  Date      Programmer  Description
  03/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "status.h"
#include "ikbbuffer.h"


BTStatus::BTStatus(BTDisplay &d, BTPc *c)
: display(d), level(0)
{
 c->serialize(this);
}

BTStatus::~BTStatus()
{
 display.clearText();
}

int BTStatus::getLevel()
{
 return level;
}

void BTStatus::startElement(const XML_Char *name, const XML_Char **atts)
{
 if (0 == strcmp("screen", name))
 {
  display.clearText();
 }
 else if (0 == strcmp("line", name))
 {
  line.clear();
 }
 else if (0 == strcmp("inventory", name))
 {
  // Special handling
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
/*    {
     std::string str(s, len);
     int index = reinterpret_cast<ValueLookup*>(state->data)->getIndex(str);
     if (-1 != index)
      reinterpret_cast<BitField*>(state->object)->set(index);
     break;
    }*/
    default:
     break;
   }
  }
 }
}

void BTStatus::endElement(const XML_Char *name)
{
 if (0 == strcmp("screen", name))
 {
  /*key = */IKeybufferGet();
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
