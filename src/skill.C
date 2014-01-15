/*-------------------------------------------------------------------------*\
  <skill.C> -- Skill implementation file

  Date      Programmer  Description
  10/11/09  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "skill.h"
#include "game.h"
#include <stdio.h>

char *use[] =
{
 "autocombat",
 "autocombat-melee",
 "autocombat-ranged",
 "combat",
 "any",
 "magic",
 "special"
};
ArrayLookup useLookup(7, use);
ArrayLookup effectLookup(BT_MONSTEREXTRADAMAGE, extraDamage);
char *specialSkill[] =
{
 "disarm",
 "hide",
 "song",
 "barehands",
 "run"
};
ArrayLookup specialLookup(5, specialSkill);

BTDice *BTSkill::getRoll(int value)
{
 if (roll.size() == 0)
  return NULL;
 else if (roll.size() > value)
  return roll[value - 1];
 else
  return roll[roll.size() - 1];
}

void BTSkill::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("use", &use, NULL, &useLookup);
 s->add("effect", &effect, NULL, &effectLookup);
 s->add("after", &after, NULL, &BTCore::getCore()->getSkillList());
 s->add("limited", &limited);
 s->add("special", &special, NULL, &specialLookup);
 s->add("roll", &roll, &BTDice::create);
 s->add("defaultDifficulty", &defaultDifficulty);
 s->add("common", &common);
 s->add("success", &success);
 s->add("failure", &failure);
}

void BTSkill::readXML(const char *filename, XMLVector<BTSkill*> &skill)
{
 XMLSerializer parser;
 parser.add("skill", &skill, &BTSkill::create);
 parser.parse(filename, true);
}

std::string BTSkillList::getName(int index)
{
 if ((size() > index) && (index >= 0))
  return operator[](index)->name;
 else
  return "---";
}

int BTSkillList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (strcmp(name.c_str(), operator[](i)->name) == 0)
   return i;
 return -1;
}

size_t BTSkillList::size()
{
 return XMLVector<BTSkill*>::size();
}

