/*-------------------------------------------------------------------------*\
  <job.C> -- Job implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "job.h"

void BTJob::serialize(XMLSerializer* s)
{
 s->add("name", &name);
 s->add("abbreviation", &abbrev);
 s->add("thac0", &thac0);
 s->add("improveThac0", &improveThac0);
 s->add("save", &save);
 s->add("improveSave", &improveSave);
 s->add("hp", &hp);
}

void BTJob::readXML(const char *filename, XMLVector<BTJob*> &job)
{
 XMLSerializer parser;
 parser.add("job", &BTJob::create, &job);
 parser.parse(filename);
}
