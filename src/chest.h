#ifndef __CHEST_H
#define __CHEST_H
/*-------------------------------------------------------------------------*\
  <chest.h> -- Chest header file

  Date      Programmer  Description
  06/09/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "display.h"
#include "dice.h"

class BTChest
{
 public:
  BTChest();
  ~BTChest();

  void clear();
  bool isSearched();
  bool isTrapped();
  void open(BTDisplay &d);
  bool removeTrap();
  void setup(char *text, int extra, const BTDice &damage, int item);
  void setSearched();

  bool searched;
  char *trapText;
  int trapExtra;
  BTDice trapDamage;
  int itemID;
};

#endif

