/*-------------------------------------------------------------------------*\
  <job.C> -- Job implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "job.h"
#include "game.h"

void BTJobSkill::serialize(ObjectSerializer* s)
{
 s->add("name", &skill, NULL, &BTGame::getGame()->getSkillList());
 s->add("value", &value);
 s->add("modifier", &modifier, NULL, &statLookup);
 s->add("improve", &improve);
}

int BTJob::calcToHit(int level)
{
 return toHit + (level / improveToHit);
}

int BTJob::calcSave(int level)
{
 return save + (level / improveSave);
}

bool BTJob::isAllowed(BTPc *pc, bool starting)
{
 if (starting)
 {
  if (advanced)
   return false;
 }
 else
 {
  // Check to see if can change class
 }
 if (allowedRace.isSet(pc->race))
 {
  return true;
 }
 return false;
}

void BTJob::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("abbreviation", &abbrev);
 s->add("picture", &picture);
 s->add("toHit", &toHit);
 s->add("improveToHit", &improveToHit);
 s->add("improveRateAttacks", &improveRateAttacks);
 s->add("maxRateAttacks", &maxRateAttacks);
 s->add("save", &save);
 s->add("improveSave", &improveSave);
 s->add("ac", &ac);
 s->add("improveAc", &improveAc);
 s->add("hp", &hp);
 s->add("skill", &skill, &BTJobSkill::create);
 s->add("xpChart", &xpChart, NULL, &BTGame::getGame()->getXpChartList());
 s->add("spells", &spells);
 s->add("advanced", &advanced);
 s->add("allowedRace", &allowedRace, &BTGame::getGame()->getRaceList());
}

void BTJob::readXML(const char *filename, XMLVector<BTJob*> &job)
{
 XMLSerializer parser;
 parser.add("job", &job, &BTJob::create);
 parser.parse(filename, true);
}

std::string BTJobList::getName(int index)
{
 if (size() > index)
  return operator[](index)->name;
 else
  return "";
}

int BTJobList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (strcmp(name.c_str(), operator[](i)->name) == 0)
   return i;
 return -1;
}

BTJobAbbrevList::BTJobAbbrevList(BTJobList *l)
 : jbList(l)
{
}

std::string BTJobAbbrevList::getName(int index)
{
 if (jbList->size() > index)
  return (*jbList)[index]->abbrev;
 else
  return "";
}

int BTJobAbbrevList::getIndex(std::string name)
{
 for (int i = 0; i < jbList->size(); ++i)
  if (strcmp(name.c_str(), (*jbList)[i]->abbrev) == 0)
   return i;
 return -1;
}

