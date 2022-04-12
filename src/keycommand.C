/*-------------------------------------------------------------------------*\
  <keycommand.C> -- Key command implementation file
\*-------------------------------------------------------------------------*/

#include "keycommand.h"
#include "game.h"

void BTKeyCommand::run(BTDisplay &d, BTSpecialContext *context)
{
 BTGame *game = BTGame::getGame();
 try
 {
  bool stop = false;
  int line = 0;
  while (!stop)
  {
   try
   {
    body.runFromLine(d, context, line);
    stop = true;
   }
   catch (const BTSpecialGoto &g)
   {
    line = body.findLabel(g.label);
   }
  }
 }
 catch (const BTSpecialStop &)
 {
 }
}

void BTKeyCommand::serialize(ObjectSerializer* s)
{
 BTCore *game = BTCore::getCore();
 s->add("key", &key);
 s->add("body", &body);
}

void BTKeyCommand::readXML(const char *filename, XMLVector<BTKeyCommand*> &keyList)
{
 XMLSerializer parser;
 parser.add("keycommand", &keyList, &BTKeyCommand::create);
 parser.parse(filename, true);
}
