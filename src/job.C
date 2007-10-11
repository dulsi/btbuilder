/*-------------------------------------------------------------------------*\
  <job.C> -- Job implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "job.h"
#include "game.h"

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
