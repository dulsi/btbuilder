/*-------------------------------------------------------------------------*\
  <status.h> -- Status implementation file

  Date      Programmer  Description
  03/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "status.h"
#include "game.h"

BTStatus::BTStatus()
{
}

BTStatus::~BTStatus()
{
}

void BTStatus::run(BTDisplay &d, BTPc *pc)
{
 setPicture(d, pc->picture, pc->name);
 setPc(pc);
 try
 {
  BTScreenSet::run(d, 0, false);
 }
 catch (const BTSpecialFlipGoForward &e)
 {
 }
 catch (const BTSpecialDead &e)
 {
  throw BTSpecialStop();
 }
 setPc(NULL);
 d.clearText();
}
