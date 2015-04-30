/*-------------------------------------------------------------------------*\
  <specialcontext.C> -- Special context implementation file

  Date      Programmer  Description
  04/28/15  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "specialcontext.h"

BTSpecialContext::BTSpecialContext()
 : grp(0)
{
 pc[0] = pc[1] = 0;
}

BTSpecialContext::~BTSpecialContext()
{
 if ((pc[0]) && (0 == pc[0]->name[0]))
  delete pc[0];
}

BTPc* BTSpecialContext::getPc()
{
 return pc[0];
}

void BTSpecialContext::setGroup(BTGroup *g)
{
 if ((pc[0]) && (0 == pc[0]->name[0]))
  delete pc[0];
 pc[0] = 0;
 grp = g;
}

void BTSpecialContext::setPc(BTPc *c, int who /*= 0*/)
{
 if ((pc[who]) && (0 == pc[who]->name[0]))
  delete pc[who];
 pc[who] = c;
 if (pc[who])
 {
  pc[who]->combat.object = -1;
  pc[who]->combat.type = BTPc::BTPcAction::none;
 }
 grp = 0;
}
