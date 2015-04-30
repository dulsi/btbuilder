#ifndef __SPECIALCONTEXT_H
#define __SPECIALCONTEXT_H
/*-------------------------------------------------------------------------*\
  <specialcontext.h> -- Special context header file

  Date      Programmer  Description
  04/27/15  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "pc.h"

class BTSpecialContext
{
 public:
  BTSpecialContext();
  ~BTSpecialContext();

  BTPc* getPc();
  void setGroup(BTGroup *g);
  void setPc(BTPc *c, int who = 0);

 protected:
  BTPc *pc[2];
  BTGroup *grp;
};

#endif
