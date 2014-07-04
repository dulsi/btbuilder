/*-------------------------------------------------------------------------*\
  <chest.C> -- Chest implementation file

  Date      Programmer  Description
  06/09/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "game.h"
#include "chest.h"

BTChest::BTChest()
: searched(false), trapText(NULL), trapDamage(0, 6, 0), trapExtra(0)
{
}

BTChest::~BTChest()
{
 if (trapText)
  delete [] trapText;
}

void BTChest::clear()
{
 searched = false;
 if (trapText)
  delete [] trapText;
 trapText = NULL;
 trapDamage.setModifier(0);
 trapDamage.setNumber(0);
 trapExtra = 0;
 itemID = BTITEM_NONE;
}

bool BTChest::isSearched()
{
 return searched;
}

bool BTChest::isTrapped()
{
 return ((trapDamage.getMax() > 0) || (trapExtra > 0));
}

void BTChest::open(BTDisplay &d)
{
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 if (isTrapped())
 {
  d.drawText("A trap on the chest is triggered!");
  if (trapText)
   d.drawText(trapText);
  for (int i = 0; i < party.size(); ++i)
  {
   if (party[i]->isAlive())
   {
    if (party[i]->savingThrow())
    {
     char tmp[100];
     snprintf(tmp, 100, "%s save!", party[i]->name);
     d.drawText(tmp);
     party[i]->takeHP(trapDamage.roll() / 2);
    }
    else
    {
     switch(trapExtra)
     {
      case BTEXTRADAMAGE_POISON:
       party[i]->status.set(BTSTATUS_POISONED);
       break;
      case BTEXTRADAMAGE_INSANITY:
       party[i]->status.set(BTSTATUS_INSANE);
       break;
      case BTEXTRADAMAGE_POSSESSION:
       party[i]->status.set(BTSTATUS_POSSESSED);
       break;
      case BTEXTRADAMAGE_PARALYSIS:
       party[i]->status.set(BTSTATUS_PARALYZED);
       break;
      default:
       break;
     }
     party[i]->takeHP(trapDamage.roll());
    }
   }
  }
 }
 party.giveItem(itemID, d);
}

bool BTChest::removeTrap()
{
 if ((trapDamage.getMax() > 0) || (trapExtra > 0))
 {
  trapDamage.setModifier(0);
  trapDamage.setNumber(0);
  trapExtra = 0;
  return true;
 }
 else
 {
  return false;
 }
}

void BTChest::setup(char *text, int extra, const BTDice &damage, int item)
{
 searched = false;
 if (trapText)
  delete [] trapText;
 trapText = new char[strlen(text) + 1];
 strcpy(trapText, text);
 trapExtra = extra;
 trapDamage = damage;
 itemID = item;
}

void BTChest::setSearched()
{
 searched = true;
}

