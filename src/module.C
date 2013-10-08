/*-------------------------------------------------------------------------*\
  <module.C> -- Module implementation file

  Date      Programmer  Description
  03/19/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "module.h"
#include "xmlserializer.h"
#include "btconst.h"
#include <string.h>

BTModule::BTModule()
 : name(NULL), author(NULL), content(NULL), startX(0), startY(0), startFace(0), maxItems(8), maxTime(14400), nightTime(7200)
{
 item = new char[strlen("DEFAULT.ITM") + 1];
 strcpy(item, "DEFAULT.ITM");
 monster = new char[strlen("DEFAULT.MON") + 1];
 strcpy(monster, "DEFAULT.MON");
 spell = new char[strlen("DEFAULT.SPL") + 1];
 strcpy(spell, "DEFAULT.SPL");
 race = new char[strlen("data/race.xml") + 1];
 strcpy(race, "data/race.xml");
 skill = new char[strlen("data/skill.xml") + 1];
 strcpy(skill, "data/skill.xml");
 song = new char[strlen("data/song.xml") + 1];
 strcpy(song, "data/song.xml");
 xpChart = new char[strlen("data/experience.xml") + 1];
 strcpy(xpChart, "data/experience.xml");
 job = new char[strlen("data/job.xml") + 1];
 strcpy(job, "data/job.xml");
 title = new char[strlen("USRSCR.LBM") + 1];
 strcpy(title, "USRSCR.LBM");
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
 if (skill)
  delete [] skill;
 if (song)
  delete [] song;
 if (xpChart)
  delete [] xpChart;
 if (job)
  delete [] job;
 if (title)
  delete [] title;
 if (wall)
  delete [] wall;
}

void BTModule::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("author", &author);
 s->add("content", &content);
 s->add("item", &item);
 s->add("monster", &monster);
 s->add("spell", &spell);
 s->add("race", &race);
 s->add("skill", &skill);
 s->add("song", &song);
 s->add("xpChart", &xpChart);
 s->add("job", &job);
 s->add("title", &title);
 s->add("wall", &wall);
 s->add("startMap", &startMap);
 s->add("startX", &startX);
 s->add("startY", &startY);
 s->add("startFace", &startFace, NULL, &directionsLookup);
 s->add("maxItems", &maxItems);
 s->add("maxTime", &maxTime);
 s->add("nightTime", &nightTime);
}
