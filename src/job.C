/*-------------------------------------------------------------------------*\
  <job.C> -- Job implementation file

  Date      Programmer  Description
  01/28/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "job.h"
#include "game.h"

bool BTJobChangeRuleAtLeast::compare(BTPc *pc)
{
 int match = 0;
 if (changeRule.size() == 0)
  return false;
 for (int i = 0; (i < changeRule.size()) && (match < minimum); ++i)
 {
  if (changeRule[i]->compare(pc))
   ++match;
 }
 return (match >= minimum);
}

void BTJobChangeRuleAtLeast::serialize(ObjectSerializer* s)
{
 s->add("changeRuleSkill", &changeRule, &BTJobChangeRuleSkill::create);
 s->add("changeRuleAtLeast", &changeRule, &BTJobChangeRuleAtLeast::create);
}

XMLObject *BTJobChangeRuleAtLeast::create(const XML_Char *name, const XML_Char **atts)
{
 BTJobChangeRuleAtLeast *obj = new BTJobChangeRuleAtLeast;
 for (const char **att = atts; *att; att += 2)
 {
  if (0 == strcmp(*att, "minimum"))
   obj->minimum = atol(att[1]);
 }
 return obj;
}

bool BTJobChangeRuleSkill::compare(BTPc *pc)
{
 if (greater != -1)
 {
  if (pc->getSkill(skill) > greater)
   return true;
 }
 if (equal != -1)
 {
  if (pc->getSkill(skill) == equal)
   return true;
 }
 return false;
}

void BTJobChangeRuleSkill::serialize(ObjectSerializer* s)
{
 s->add("skill", &skill, NULL, &BTCore::getCore()->getSkillList());
 s->add("greater", &greater);
 s->add("equal", &equal);
}

void BTJobSkillPurchase::serialize(ObjectSerializer* s)
{
 s->add("minimumLevel", &minimumLevel);
 s->add("value", &value);
 s->add("cost", &cost);
}

BTJobSkillPurchase *BTJobSkill::findNextPurchase(int current)
{
 BTJobSkillPurchase *found = NULL;
 for (int pr = 0; pr < purchase.size(); ++pr)
 {
  if ((current < purchase[pr]->value) && ((found == NULL) || (purchase[pr]->value < found->value)))
  {
   found = purchase[pr];
  }
 }
 return found;
}

void BTJobSkill::serialize(ObjectSerializer* s)
{
 s->add("name", &skill, NULL, &BTCore::getCore()->getSkillList());
 s->add("value", &value);
 s->add("modifier", &modifier, NULL, &statLookup);
 s->add("improve", &improve);
 s->add("improveLevel", &improveLevel);
 s->add("purchase", &purchase, &BTJobSkillPurchase::create);
}

int BTJob::calcToHit(int level)
{
 return toHit + (level / improveToHit);
}

int BTJob::calcSave(int level)
{
 return save + (level / improveSave);
}

BTJobSkill *BTJob::getSkill(int skillIndex)
{
 for (int i = 0; i < skill.size(); ++i)
 {
  if (skill[i]->skill == skillIndex)
   return skill[i];
 }
 return NULL;
}

bool BTJob::isAllowed(BTPc *pc, bool starting)
{
 if (starting)
 {
  if (advanced)
   return false;
 }
 else
 {
  // Check to see if can change class
  if (changeRule.size() == 0)
   return false;
  for (int i = 0; i < changeRule.size(); ++i)
  {
   if (!changeRule[i]->compare(pc))
    return false;
  }
 }
 if (allowedRace.isSet(pc->race))
 {
  return true;
 }
 return false;
}

void BTJob::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("abbreviation", &abbrev);
 s->add("malePicture", &malePicture);
 s->add("femalePicture", &femalePicture);
 s->add("toHit", &toHit);
 s->add("improveToHit", &improveToHit);
 s->add("improveRateAttacks", &improveRateAttacks);
 s->add("maxRateAttacks", &maxRateAttacks);
 s->add("save", &save);
 s->add("improveSave", &improveSave);
 s->add("ac", &ac);
 s->add("improveAc", &improveAc);
 s->add("hp", &hp);
 s->add("skill", &skill, &BTJobSkill::create);
 s->add("changeRuleSkill", &changeRule, &BTJobChangeRuleSkill::create);
 s->add("changeRuleAtLeast", &changeRule, &BTJobChangeRuleAtLeast::create);
 s->add("xpChart", &xpChart, NULL, &BTCore::getCore()->getXpChartList());
 s->add("spells", &spells);
 s->add("advanced", &advanced);
 s->add("allowedRace", &allowedRace, &BTCore::getCore()->getRaceList());
}

void BTJob::readXML(const char *filename, XMLVector<BTJob*> &job)
{
 XMLSerializer parser;
 parser.add("job", &job, &BTJob::create);
 parser.parse(filename, true);
}

std::string BTJobList::getName(int index)
{
 if (size() > index)
  return operator[](index)->name;
 else
  return "";
}

int BTJobList::getIndex(std::string name)
{
 for (int i = 0; i < size(); ++i)
  if (strcmp(name.c_str(), operator[](i)->name) == 0)
   return i;
 return -1;
}

size_t BTJobList::size()
{
 return XMLVector<BTJob*>::size();
}

BTJobAbbrevList::BTJobAbbrevList(BTJobList *l)
 : jbList(l)
{
}

std::string BTJobAbbrevList::getName(int index)
{
 if (jbList->size() > index)
  return (*jbList)[index]->abbrev;
 else
  return "";
}

int BTJobAbbrevList::getIndex(std::string name)
{
 for (int i = 0; i < jbList->size(); ++i)
  if (strcmp(name.c_str(), (*jbList)[i]->abbrev) == 0)
   return i;
 return -1;
}

size_t BTJobAbbrevList::size()
{
 return jbList->size();
}

