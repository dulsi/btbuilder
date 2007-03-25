/*-------------------------------------------------------------------------*\
  <job.C> -- Job implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "job.h"
#include "game.h"

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

void BTJob::serialize(XMLSerializer* s)
{
 s->add("name", &name);
 s->add("abbreviation", &abbrev);
 s->add("picture", &picture);
 s->add("thac0", &thac0);
 s->add("improveThac0", &improveThac0);
 s->add("save", &save);
 s->add("improveSave", &improveSave);
 s->add("hp", &hp);
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
