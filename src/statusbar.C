/*-------------------------------------------------------------------------*\
  <statusbar.h> -- Status bar implementation file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "statusbar.h"
#include "game.h"

void BTStatusBar::draw()
{
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
 XMLVector<BTStatusInfo*> &statusInfo = display.getConfig()->statusInfo;
 if (statusInfo.size() == 0)
  return;
 for (int i = 0; i < party.size(); ++i)
 {
  SDL_Rect &dst = display.getConfig()->status[i];
  party[i]->serialize(this);
  for (int k = 0; k < statusInfo.size(); ++k)
  {
   statusInfo[k]->draw(display, dst.x, dst.y, this);
  }
  removeLevel();
 }
}

int BTStatusBar::getLevel()
{
 return 0;
}
