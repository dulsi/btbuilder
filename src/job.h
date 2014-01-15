#ifndef __JOB_H
#define __JOB_H
/*-------------------------------------------------------------------------*\
  <job.h> -- Job header file

  Date      Programmer  Description
  01/26/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "xmlserializer.h"
#include "pc.h"

class BTJobChangeRule : public XMLObject
{
 public:
  virtual bool compare(BTPc *pc) = 0;
};

class BTJobChangeRuleAtLeast : public BTJobChangeRule
{
 public:
  BTJobChangeRuleAtLeast() : minimum(0) {}

  virtual bool compare(BTPc *pc);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts);

  int minimum;
  XMLVector<BTJobChangeRule*> changeRule;
};

class BTJobChangeRuleSkill : public BTJobChangeRule
{
 public:
  BTJobChangeRuleSkill() : skill(-1), greater(-1), equal(-1) {}

  virtual bool compare(BTPc *pc);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTJobChangeRuleSkill; }

  int skill;
  int greater;
  int equal;
};

class BTJobSkillPurchase : public XMLObject
{
 public:
  BTJobSkillPurchase() : minimumLevel(0), value(0), cost(0) {}

  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTJobSkillPurchase; }

  int minimumLevel;
  int value;
  int cost;
};

class BTJobSkill : public XMLObject
{
 public:
  BTJobSkill() : skill(-1), value(0), modifier(-1), improve(0), improveLevel(1) {}

  BTJobSkillPurchase *findNextPurchase(int current);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTJobSkill; }

  int skill;
  int value;
  int modifier;
  int improve;
  int improveLevel;
  XMLVector<BTJobSkillPurchase*> purchase;
};

class BTJob : public XMLObject
{
 public:
  BTJob()
   : malePicture(-1), femalePicture(-1), toHit(0), improveToHit(0), improveRateAttacks(0), maxRateAttacks(0), save(0), improveSave(0), ac(0), improveAc(0), hp(0), xpChart(-1), spells(false), advanced(false)
  {
   name = new char[1];
   name[0] = 0;
   abbrev = new char[1];
   abbrev[0] = 0;
  }

  ~BTJob() { delete [] name; delete [] abbrev; }

  int calcToHit(int level);
  int calcSave(int level);
  BTJobSkill *getSkill(int skillIndex);
  bool isAllowed(BTPc *pc, bool starting);
  virtual void serialize(ObjectSerializer* s);

  static XMLObject *create(const XML_Char *name, const XML_Char **atts) { return new BTJob; }
  static void readXML(const char *filename, XMLVector<BTJob*> &job);

  char *name;
  char *abbrev;
  int malePicture;
  int femalePicture;
  int toHit;
  int improveToHit;
  int improveRateAttacks;
  int maxRateAttacks;
  int save;
  int improveSave;
  int ac;
  int improveAc;
  int hp;
  XMLVector<BTJobSkill*> skill;
  XMLVector<BTJobChangeRule*> changeRule;
  int xpChart;
  bool spells;
  bool advanced;
  BitField allowedRace;
};

class BTJobList : public ValueLookup, public XMLVector<BTJob*>
{
 public:
  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();
};

class BTJobAbbrevList : public ValueLookup
{
 public:
  BTJobAbbrevList(BTJobList *l);

  virtual std::string getName(int index);
  virtual int getIndex(std::string name);
  virtual size_t size();

 private:
  BTJobList *jbList;
};

#endif
