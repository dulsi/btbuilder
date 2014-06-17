#ifndef __EFFECTGROUP_H
#define __EFFECTGROUP_H
/*-------------------------------------------------------------------------*\
  <effectgroup.h> -- Effect group header file

  Date      Programmer  Description
  10/03/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spelleffect.h"

class BTEffectGroup
{
 public:
  BTEffectGroup();
  ~BTEffectGroup();

  virtual void addEffect(BTBaseEffect *e);
  virtual void checkExpiration(BTDisplay &d, BTCombat *combat = NULL);
  virtual void clearEffects(BTDisplay &d);
  virtual void clearEffectsByType(BTDisplay &d, int type);
  virtual void clearEffectsBySource(BTDisplay &d, bool song, int group = BTTARGET_NONE, int target = BTTARGET_INDIVIDUAL);
  virtual void clearMapEffects();
  virtual bool hasEffectOfType(int type, int group = BTTARGET_NONE, int target = BTTARGET_INDIVIDUAL, bool exact = false);
  virtual void addPlayer(BTDisplay &d, int who);
  virtual void movedPlayer(BTDisplay &d, BTCombat *combat, int who, int where);

 protected:
  void checkMusic(BTDisplay &d, std::vector<int> &musicIds);
  virtual void maintain(BTDisplay &d, BTCombat *combat = NULL);

 protected:
  XMLVector<BTBaseEffect*> effect;
};

#endif

