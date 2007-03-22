/*-------------------------------------------------------------------------*\
  <module.C> -- Module implementation file

  Date      Programmer  Description
  03/19/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "module.h"
#include "xmlserializer.h"

BTModule::BTModule()
 : name(NULL), author(NULL), content(NULL)
{
 item = new char[strlen("DEFAULT.ITM") + 1];
 strcpy(item, "DEFAULT.ITM");
 monster = new char[strlen("DEFAULT.MON") + 1];
 strcpy(monster, "DEFAULT.MON");
 spell = new char[strlen("DEFAULT.SPL") + 1];
 strcpy(spell, "DEFAULT.SPL");
 race = new char[strlen("data/race.xml") + 1];
 strcpy(race, "data/race.xml");
 job = new char[strlen("data/job.xml") + 1];
 strcpy(job, "data/job.xml");
 start = new char[strlen("START.BRD") + 1];
 strcpy(start, "START.BRD");
 title = new char[strlen("USRSCR.LBM") + 1];
 strcpy(title, "USRSCR.LBM");
 background = new char[strlen("image/mainscreen.png") + 1];
 strcpy(background, "image/mainscreen.png");
 wall = new char[strlen("data/wall.xml") + 1];
 strcpy(wall, "data/wall.xml");
}

BTModule::~BTModule()
{
 if (name)
  delete [] name;
 if (author)
  delete [] author;
 if (content)
  delete [] content;
 if (item)
  delete [] item;
 if (monster)
  delete [] monster;
 if (spell)
  delete [] spell;
 if (race)
  delete [] race;
 if (job)
  delete [] job;
 if (start)
  delete [] start;
 if (title)
  delete [] title;
 if (background)
  delete [] background;
 if (wall)
  delete [] wall;
}

void BTModule::serialize(XMLSerializer* s)
{
 s->add("name", &name);
 s->add("author", &author);
 s->add("content", &content);
 s->add("item", &item);
 s->add("monster", &monster);
 s->add("spell", &spell);
 s->add("race", &race);
 s->add("job", &job);
 s->add("start", &start);
 s->add("title", &title);
}
