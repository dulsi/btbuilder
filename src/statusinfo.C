/*-------------------------------------------------------------------------*\
  <statusinfo.C> -- Status information implementation file

  Date      Programmer  Description
  04/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "statusinfo.h"
#include "display.h"

void BTStatBlock::serialize(ObjectSerializer* s)
{
 s->add("attribute", &attribute);
 s->add("position", &position);
 s->add("modifier", &modifier);
 s->add("negate", &negate);
 s->add("max", &maxValue);
 s->add("overflow", &overflow);
 s->add("align", &align, NULL, &BTAlignmentLookup::lookup);
}

void BTStatBlock::draw(BTDisplay &d, int x, int y, ObjectSerializer *pc)
{
 int xMult, yMult;
 SDL_Rect dst;
 d.getMultiplier(xMult, yMult);
 dst.x = (x + position.x) * xMult;
 dst.y = (y + position.y) * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 XMLAction *state = pc->find(attribute, NULL);
 if (state)
 {
  switch(state->type)
  {
   case XMLTYPE_BOOL:
    if (*(reinterpret_cast<bool*>(state->object)))
     d.drawFont("true", dst, d.getBlack(), (BTDisplay::alignment)align);
    else
     d.drawFont("false", dst, d.getBlack(), (BTDisplay::alignment)align);
    break;
   case XMLTYPE_INT:
    if (state->data)
    {
     d.drawFont(reinterpret_cast<ValueLookup*>(state->data)->getName(*(reinterpret_cast<int*>(state->object))).c_str(), dst, d.getBlack(), (BTDisplay::alignment)align);
    }
    else
    {
     int val = *(reinterpret_cast<int*>(state->object)) + modifier;
     if (negate)
      val *= -1;
     if ((maxValue != -1) && (maxValue < val))
     {
      d.drawFont(overflow, dst, d.getBlack(), (BTDisplay::alignment)align);
     }
     else
     {
      char tmp[40];
      snprintf(tmp, 40, "%d", val);
      d.drawFont(tmp, dst, d.getBlack(), (BTDisplay::alignment)align);
     }
    }
    break;
   case XMLTYPE_UINT:
   {
/*    char tmp[40];
    snprintf(tmp, 40, "%u", *(reinterpret_cast<unsigned int*>(state->object)));
    line.back() += tmp;*/
    break;
   }
   case XMLTYPE_STRING:
    d.drawFont(*(reinterpret_cast<char**>(state->object)), dst, d.getBlack(), (BTDisplay::alignment)align);
    break;
   case XMLTYPE_BITFIELD:
   default:
    break;
  }
 }
}

BTAlignmentLookup BTAlignmentLookup::lookup;
char *BTAlignmentLookup::value[3] = { "left", "center", "right" };

std::string BTAlignmentLookup::getName(int index)
{
 return value[index];
}

int BTAlignmentLookup::getIndex(std::string name)
{
 for (int i = 0; i < 3; ++i)
 {
  if (0 == strcmp(name.c_str(), value[i]))
   return i;
 }
 return 0;
}
