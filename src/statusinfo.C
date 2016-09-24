/*-------------------------------------------------------------------------*\
  <statusinfo.C> -- Status information implementation file

  Date      Programmer  Description
  04/02/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "statusinfo.h"
#include "display.h"

void BTStatCompare::serialize(ObjectSerializer* s)
{
 s->add("attribute", &attribute);
 s->add("full", &full);
 s->add("half", &half);
}

void BTStatBlock::serialize(ObjectSerializer* s)
{
 s->add("attribute", &attribute);
 s->add("position", &position);
 s->add("modifier", &modifier);
 s->add("negate", &negate);
 s->add("max", &maxValue);
 s->add("overflow", &overflow);
 s->add("align", &align, NULL, &BTAlignmentLookup::lookup);
 s->add("compare", &compare);
}

void BTStatBlock::draw(BTBackgroundAndScreen &d, int x, int y, ObjectSerializer *pc)
{
 int xMult, yMult;
 SDL_Rect dst;
 std::string color("black");
 d.getDisplay()->getMultiplier(xMult, yMult);
 dst.x = (x + position.x) * xMult;
 dst.y = (y + position.y) * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 XMLAction *state = pc->find(attribute, NULL);
 if (state)
 {
  switch(state->getType())
  {
   case XMLTYPE_BOOL:
    if (*(reinterpret_cast<bool*>(state->object)))
     d.drawFont("true", dst, d.getColor(color), (BTDisplay::alignment)align);
    else
     d.drawFont("false", dst, d.getColor(color), (BTDisplay::alignment)align);
    break;
   case XMLTYPE_INT:
    if (compare.attribute != "")
    {
     XMLAction *cmpState = pc->find(compare.attribute.c_str(), NULL);
     if ((cmpState) && (cmpState->getType() == XMLTYPE_INT))
     {
      int curVal = *(reinterpret_cast<int*>(state->object));
      int maxVal = *(reinterpret_cast<int*>(cmpState->object));
      if (maxVal == curVal)
       color = compare.full;
      else if (curVal < (maxVal / 2))
       color = compare.half;
     }
    }
    if (state->data)
    {
     d.drawFont(reinterpret_cast<ValueLookup*>(state->data)->getName(*(reinterpret_cast<int*>(state->object))).c_str(), dst, d.getColor(color), (BTDisplay::alignment)align);
    }
    else
    {
     int val = *(reinterpret_cast<int*>(state->object)) + modifier;
     if (negate)
      val *= -1;
     if ((maxValue != -1) && (maxValue < val))
     {
      d.drawFont(overflow, dst, d.getColor(color), (BTDisplay::alignment)align);
     }
     else
     {
      char tmp[40];
      snprintf(tmp, 40, "%d", val);
      d.drawFont(tmp, dst, d.getColor(color), (BTDisplay::alignment)align);
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
    d.drawFont(*(reinterpret_cast<char**>(state->object)), dst, d.getColor(color), (BTDisplay::alignment)align);
    break;
   case XMLTYPE_BITFIELD:
   default:
    break;
  }
 }
}

void BTPrint::serialize(ObjectSerializer* s)
{
 s->add("text", &text);
 s->add("position", &position);
 s->add("align", &align, NULL, &BTAlignmentLookup::lookup);
 s->add("color", &color);
}

void BTPrint::draw(BTBackgroundAndScreen &d, int x, int y, ObjectSerializer *pc)
{
 int xMult, yMult;
 SDL_Rect dst;
 d.getDisplay()->getMultiplier(xMult, yMult);
 dst.x = (x + position.x) * xMult;
 dst.y = (y + position.y) * yMult;
 dst.w = position.w * xMult;
 dst.h = position.h * yMult;
 d.drawFont(text, dst, d.getColor(color), (BTDisplay::alignment)align);
}

bool BTCondition::compare(ObjectSerializer *pc) const
{
 return true;
}

void BTCondition::draw(BTBackgroundAndScreen &d, int x, int y, ObjectSerializer *pc)
{
 for (int i = 0; i < info.size(); ++i)
 {
  info[i]->draw(d, x, y, pc); 
 }
}

void BTCondition::serialize(ObjectSerializer* s)
{
 s->add("statBlock", &info, &BTStatBlock::create);
 s->add("conditional", &info, &BTConditional::create);
 s->add("print", &info, &BTPrint::create);
}

bool BTCheckBit::compare(ObjectSerializer *pc) const
{
 XMLAction *state = pc->find(attribute, NULL);
 if ((state) && (XMLTYPE_BITFIELD == state->getType()))
 {
  return reinterpret_cast<BitField*>(state->object)->isSet(bit);
 }
 return false;
}

void BTCheckBit::serialize(ObjectSerializer* s)
{
 s->add("attribute", &attribute);
 s->add("bit", &bit);
 BTCondition::serialize(s);
}

void BTConditional::draw(BTBackgroundAndScreen &d, int x, int y, ObjectSerializer *pc)
{
 for (int i = 0; i < condition.size(); ++i)
 {
  if (condition[i]->compare(pc))
  {
   condition[i]->draw(d, x, y, pc);
   return;
  }
 }
}

void BTConditional::serialize(ObjectSerializer* s)
{
 s->add("checkBit", &condition, &BTCheckBit::create);
 s->add("default", &condition, &BTCondition::create);
}

BTAlignmentLookup BTAlignmentLookup::lookup;
const char *BTAlignmentLookup::value[3] = { "left", "center", "right" };
