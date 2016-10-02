/*-------------------------------------------------------------------------*\
  <statusbar.h> -- Status bar implementation file

  Date      Programmer  Description
  04/01/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "statusbar.h"
#include "game.h"

void BTStatusBar::draw(BTBackgroundAndScreen *display)
{
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
 XMLVector<BTStatusInfo*> &statusInfo = display->getDisplay()->getConfig()->statusInfo;
 if (statusInfo.size() == 0)
  return;
 SDL_Rect dst;
 int i;
 int xMult, yMult;
 display->getDisplay()->getMultiplier(xMult, yMult);
 for (i = 0; i < BT_PARTYSIZE; ++i)
 {
  SDL_Rect &dstOrig = display->getDisplay()->getConfig()->status[i];
  dst.x = dstOrig.x * xMult;
  dst.y = dstOrig.y * yMult;
  dst.w = dstOrig.w * xMult;
  dst.h = dstOrig.h * yMult;
  display->clear(dst);
 }
 for (i = 0; i < party.size(); ++i)
 {
  SDL_Rect &dst = display->getDisplay()->getConfig()->status[i];
  party[i]->serialize(this);
  for (int k = 0; k < statusInfo.size(); ++k)
  {
   statusInfo[k]->draw(*display, dst.x, dst.y, this);
  }
  removeLevel();
 }
}

int BTStatusBar::getLevel()
{
 return 0;
}
