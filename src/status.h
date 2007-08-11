#ifndef __STATUS_H
#define __STATUS_H
/*-------------------------------------------------------------------------*\
  <status.h> -- Status header file

  Date      Programmer  Description
  03/27/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "screenset.h"
#include "display.h"
#include "pc.h"

class BTStatus : public BTScreenSet
{
 public:
  BTStatus();
  ~BTStatus();

  void run(BTDisplay &d, BTPc *pc);
};

#endif
