/*-------------------------------------------------------------------------*\
  <effectgroup.C> -- Effect group implementation file

  Date      Programmer  Description
  10/03/12  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "effectgroup.h"
#include "game.h"

BTEffectGroup::BTEffectGroup()
{
}

BTEffectGroup::~BTEffectGroup()
{
}

void BTEffectGroup::addEffect(BTBaseEffect *e)
{
 effect.push_back(e);
}

void BTEffectGroup::clearEffects(BTDisplay &d)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  BTBaseEffect *current = *itr;
  current->expire = true;
 }
}

void BTEffectGroup::clearEffectsByType(BTDisplay &d, int type)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->type == type)
  {
   BTBaseEffect *current = *itr;
   current->expire = true;
  }
 }
}

void BTEffectGroup::clearEffectsBySource(BTDisplay &d, bool song, int group /*= BTTARGET_NONE*/, int target /*= BTTARGET_INDIVIDUAL*/)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (((song == true) && ((*itr)->singer != BTTARGET_NOSINGER)) || ((song == false) && ((*itr)->singer == BTTARGET_NOSINGER)))
  {
   BTBaseEffect *current = *itr;
   bool exact = (target != BTTARGET_INDIVIDUAL);
   if ((group == BTTARGET_NONE) || (current->targets(group, target, exact)))
   {
    current->expire = true;
   }
  }
 }
}

void BTEffectGroup::clearMapEffects()
{
 bool bFound = true;
 while (bFound)
 {
  bFound = false;
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
  {
   if (BTTIME_MAP == (*itr)->expiration)
   {
    BTBaseEffect *current = *itr;
    effect.erase(itr, false);
//    current->finish(d, NULL);
    delete current;
    bFound = true;
    break;
   }
  }
 }
}

bool BTEffectGroup::hasEffectOfType(int type, int group /*= BTTARGET_NONE*/, int target /*= BTTARGET_INDIVIDUAL*/, bool exact /*= false*/)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->type == type)
  {
   if (group != BTTARGET_NONE)
   {
    if ((*itr)->targets(group, target, false))
     return true;
   }
   else
    return true;
  }
 }
 return false;
}

void BTEffectGroup::addPlayer(BTDisplay &d, int who)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->targets(BTTARGET_PARTY, BTTARGET_INDIVIDUAL))
  {
   (*itr)->apply(d, NULL, BTTARGET_PARTY, who);
  }
 }
}

void BTEffectGroup::movedPlayer(BTDisplay &d, BTCombat *combat, int who, int where)
{
 if (where == BTPARTY_REMOVE)
 {
  for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
  {
   if ((*itr)->targets(BTTARGET_PARTY, who))
   {
    BTBaseEffect *current = *itr;
    current->expire = true;
   }
   ++itr;
  }
 }
 checkExpiration(d, NULL);
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if (BTPARTY_REMOVE == where)
   (*itr)->remove(combat, BTTARGET_PARTY, where);
  else
   (*itr)->move(BTTARGET_PARTY, who, where);
 }
}

void BTEffectGroup::checkExpiration(BTDisplay &d, BTCombat *combat /*= NULL*/)
{
 std::vector<int> musicIds;
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
 {
  if ((*itr)->isExpired(BTGame::getGame()))
  {
   BTBaseEffect *current = *itr;
   itr = effect.erase(itr, false);
   bool musicFound = false;
   for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
   {
    if (*itrId == current->musicId)
     musicFound = true;
   }
   if (!musicFound)
    musicIds.push_back(current->musicId);
   int size = effect.size();
   if ((BTTIME_PERMANENT != current->expiration) && (BTTIME_CONTINUOUS != current->expiration))
    current->finish(d, combat);
   delete current;
   if (size != effect.size())
    itr = effect.begin();
  }
  else
   ++itr;
 }
 checkMusic(d, musicIds);
}

void BTEffectGroup::checkMusic(BTDisplay &d, std::vector<int> &musicIds)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end(); ++itr)
 {
  if ((*itr)->musicId != BTMUSICID_NONE)
  {
   for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
   {
    if (*itrId == (*itr)->musicId)
    {
     musicIds.erase(itrId);
    }
   }
  }
 }
 for (std::vector<int>::iterator itrId = musicIds.begin(); itrId != musicIds.end(); ++itrId)
 {
  d.stopMusic(*itrId);
 }
}

void BTEffectGroup::maintain(BTDisplay &d, BTCombat *combat /*= NULL*/)
{
 for (XMLVector<BTBaseEffect*>::iterator itr = effect.begin(); itr != effect.end();)
 {
  if ((*itr)->first)
   (*itr)->first = false;
  else if (BTTIME_PERMANENT != (*itr)->expiration)
   (*itr)->maintain(d, combat);
  ++itr;
 }
 checkExpiration(d, combat);
}

