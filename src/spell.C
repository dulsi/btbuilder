/*-------------------------------------------------------------------------*\
  <spell.C> -- Spell implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spell.h"
#include "game.h"

int BTSpell::version(1);

BTSpell::BTSpell()
 : caster(0), level(1), sp(1), range(0), effectiveRange(0), area(BTAREAEFFECT_NONE), duration(BTDURATION_ONE)
{
 code = new char[1];
 code[0] = 0;
 effect = new char[1];
 effect[0] = 0;
}

BTSpell::BTSpell(const BTSpell &copy)
 : name(copy.name), caster(copy.caster), level(copy.level), sp(copy.sp), range(copy.range), effectiveRange(copy.effectiveRange),
 area(copy.area), duration(copy.duration)
{
 code = new char[strlen(copy.code) + 1];
 strcpy(code, copy.code);
 effect = new char[strlen(copy.effect) + 1];
 strcpy(effect, copy.effect);
}

BTSpell::~BTSpell()
{
 if (code)
  delete [] code;
 if (effect)
  delete [] effect;
}

std::string BTSpell::describeManifest() const
{
 std::string s;
 for (auto itr = manifest.begin(); itr != manifest.end(); itr++)
 {
  s += (*itr)->createString();
  s += "\n";
 }
 return s;
}

const std::string &BTSpell::getName() const
{
 return name;
}

int BTSpell::getArea() const
{
 return area;
}

int BTSpell::getCaster() const
{
 return caster;
}

const char *BTSpell::getCode() const
{
 return code;
}

int BTSpell::getDuration() const
{
 return duration;
}

const char *BTSpell::getEffect() const
{
 return effect;
}

IShort BTSpell::getEffectiveRange() const
{
 return effectiveRange;
}

IShort BTSpell::getLevel() const
{
 return level;
}

IShort BTSpell::getRange() const
{
 return range;
}

IShort BTSpell::getSp() const
{
 return sp;
}

void BTSpell::write(BinaryWriteFile &f)
{
 IUByte unknown = 0x00;
 IShort num;
 IShort extra(0), type(0);
 char tmp[29];
 BTDice dice(0, 2);

 strncpy(tmp, name.c_str(), 29);
 f.writeUByteArray(29, (IUByte *)tmp);
 strncpy(tmp, code, 5);
 f.writeUByteArray(5, (IUByte *)tmp);
 num = caster;
 f.writeShort(num);
 f.writeShort(level);
 f.writeShort(sp);
 f.writeShort(range);
 f.writeShort(effectiveRange);
 if (manifest.size() != 1)
  throw FileException("Multi effect spells not supported in older file format.");
 manifest[0]->supportOldFormat(type, dice, extra);
 f.writeShort(type);
 num = area;
 f.writeShort(num);
 dice.write(f);
 f.writeUByte(unknown);
 num = duration;
 f.writeShort(num);
 f.writeShort(extra);
 strncpy(tmp, effect, 22);
 f.writeUByteArray(22, (IUByte *)tmp);
}

int BTSpell::activate(BTDisplay &d, const char *activation, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target)
{
 int killed = 0;
 BTGame *game = BTGame::getGame();
 BTParty &party = game->getParty();
 unsigned int expire = 0;
 switch(duration)
 {
  case BTDURATION_ONE:
   expire = game->getExpiration(1);
   break;
  case BTDURATION_SHORT:
   expire = game->getExpiration(BTDice(1, 121, 119).roll());
   break;
  case BTDURATION_MEDIUM:
   expire = game->getExpiration(BTDice(1, 181, 179).roll());
   break;
  case BTDURATION_LONG:
   expire = game->getExpiration(BTDice(1, 241, 239).roll());
   break;
  case BTDURATION_COMBAT:
   expire = BTTIME_COMBAT;
   break;
  case BTDURATION_PERMANENT:
   expire = BTTIME_PERMANENT;
   break;
  case BTDURATION_CONTINUOUS:
   expire = BTTIME_CONTINUOUS;
   break;
  case BTDURATION_INDEFINITE:
   expire = BTTIME_INDEFINITE;
   break;
  default:
   break;
 }
 std::string text = activation;
 if (text.length() > 0)
  text += " ";
 text += effect;
 switch(area)
 {
  case BTAREAEFFECT_GROUP:
   if (BTTARGET_PARTY == group)
    text += " the whole party!";
   else
   {
    BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
    BTFactory<BTMonster> &monList = game->getMonsterList();
    text += " ";
    if (grp->size() > 1)
     text += monList[grp->monsterType].getName();
    else
     text += monList[grp->monsterType].getPluralName();
    text += ".";
   }
   break;
  case BTAREAEFFECT_FOE:
   text += " ";
   if (BTTARGET_PARTY == group)
   {
    text += party[target]->name;
   }
   else
   {
    BTMonsterGroup *grp = combat->getMonsterGroup(group - BTTARGET_MONSTER);
    BTFactory<BTMonster> &monList = game->getMonsterList();
    text += monList[grp->monsterType].getName();
   }
   text += ".";
  case BTAREAEFFECT_NONE:
  default:
   break;
 }
 d.drawMessage(text.c_str(), game->getDelay());
 for (int i = 0; i < manifest.size(); ++i)
 {
  std::list<BTBaseEffect*> effect = manifest[i]->manifest(d, partySpell, combat, expire, casterLevel, distance, group, target, BTTARGET_NOSINGER, BTMUSICID_NONE);
  for (auto itr = effect.begin(); itr != effect.end(); itr++)
  {
   try
   {
    killed = (*itr)->apply(d, combat);
    game->addEffect((*itr));
   }
   catch (const BTAllResistException &e)
   {
    delete (*itr);
   }
  }
 }
 return killed;
}

int BTSpell::cast(BTDisplay &d, const char *caster, int casterGroup, int casterTarget, bool partySpell, BTCombat *combat, int casterLevel, int distance, int group, int target)
{
 std::string text = caster;
 BTGame *game = BTGame::getGame();
 if (game->getFlags().isSet(BTSPECIALFLAG_ANTIMAGIC))
 { 
  text += " cannot seem to cast ";
  text += name;
  text += ".";
  d.drawMessage(text.c_str(), game->getDelay());
  return 0;
 }
 else if (game->hasEffectOfType(BTSPELLTYPE_BLOCKMAGIC, casterGroup, casterTarget))
 {
  text += " gestures. ";
  text += caster;
  text += "'s spell shatters.";
  d.drawMessage(text.c_str(), game->getDelay());
  return 0;
 }
 else
 {
  text += " casts ";
  text += name;
  text += ".";
  return activate(d, text.c_str(), partySpell, combat, casterLevel, distance, group, target);
 }
}

void BTSpell::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("code", &code);
 s->add("caster", &caster, NULL, &BTCore::getCore()->getSkillList());
 s->add("level", &level);
 s->add("sp", &sp);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("area", &area, NULL, &areaLookup);
 s->add("duration", &duration, NULL, &durationLookup);
 s->add("effect", &effect);
 BTManifest::serializeSetup(s, manifest);
}

void BTSpell::upgrade()
{
}

XMLObject *BTSpell::create(const XML_Char *name, const XML_Char **atts)
{
 return ((version == 2) ? new BTSpell : new BTSpell1);
}

void BTSpell::readXML(const char *filename, XMLVector<BTSpell*> &spell)
{
 XMLSerializer parser;
 version = 1;
 parser.add("version", &version);
 parser.add("spell", &spell, &BTSpell::create);
 parser.parse(filename, true);
 if (version == 1)
 {
  for (int i = 0; i < spell.size(); ++i)
   spell[i]->upgrade();
 }
}

void BTSpell::writeXML(const char *filename, XMLVector<BTSpell*> &spell)
{
 XMLSerializer parser;
 version = 2;
 parser.add("version", &version);
 parser.add("spell", &spell, &BTSpell::create);
 parser.write(filename, true);
}

BTSpell1::BTSpell1(BinaryReadFile &f)
{
 IShort num;
 IUByte tmp[30];

 f.readUByteArray(29, tmp);
 tmp[29] = 0;
 name = (char *)tmp;
 f.readUByteArray(5, tmp);
 tmp[5] = 0;
 code = new char[strlen((char *)tmp) + 1];
 strcpy(code, (char *)tmp);
 f.readShort(num);
 caster = num;
 f.readShort(level);
 f.readShort(sp);
 f.readShort(range);
 f.readShort(effectiveRange);
 f.readShort(num);
 type = num;
 f.readShort(num);
 area = num;
 dice.read(f);
 f.readUByte(tmp[0]);
 f.readShort(num);
 duration = num;
 f.readShort(num);
 extra = num;
 f.readUByteArray(22, tmp);
 tmp[22] = 0;
 effect = new char[strlen((char *)tmp) + 1];
 strcpy(effect, (char *)tmp);
}

BTSpell1::BTSpell1()
 : type(BTSPELLTYPE_HEAL), extra(0)
{
}

BTSpell1::BTSpell1(const BTSpell1 &copy)
 : BTSpell(copy), type(copy.type), dice(copy.dice), extra(copy.extra)
{
}

void BTSpell1::serialize(ObjectSerializer* s)
{
 BTSpell::serialize(s);
 s->add("type", &type, NULL, &spellTypeLookup);
 s->add("dice", &dice);
 s->add("extra", &extra);
 s->add("extraMonster", &extra, NULL, &BTCore::getCore()->getMonsterList());
}

BTSpell1::~BTSpell1()
{
}

const BTDice &BTSpell1::getDice() const
{
 return dice;
}

int BTSpell1::getExtra() const
{
 return extra;
}

int BTSpell1::getType() const
{
 return type;
}

void BTSpell1::upgrade()
{
 if (manifest.size() == 0)
 {
  switch (type)
  {
   case BTSPELLTYPE_LIGHT:
    manifest.push_back(new BTLightManifest());
    break;
   case BTSPELLTYPE_DOORDETECT:
   case BTSPELLTYPE_TRAPDESTROY:
   case BTSPELLTYPE_COMPASS:
   case BTSPELLTYPE_BLOCKENCOUNTERS:
   case BTSPELLTYPE_LEVITATION:
    manifest.push_back(new BTManifest(type));
    break;
   case BTSPELLTYPE_ARMORBONUS:
   case BTSPELLTYPE_ATTACKRATEBONUS:
   case BTSPELLTYPE_SAVEBONUS:
   case BTSPELLTYPE_HITBONUS:
    manifest.push_back(new BTBonusManifest(type, getExtra()));
    break;
   case BTSPELLTYPE_DAMAGE:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), dice, BTEXTRADAMAGE_NONE, 0));
    break;
   case BTSPELLTYPE_KILL:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_CRITICALHIT, 0));
    break;
   case BTSPELLTYPE_POISON:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_POISON, 0));
    break;
   case BTSPELLTYPE_CAUSEINSANITY:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_INSANITY, 0));
    break;
   case BTSPELLTYPE_POSSESS:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_POSSESSION, 0));
    break;
   case BTSPELLTYPE_FLESHTOSTONE:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_STONED, 0));
    break;
   case BTSPELLTYPE_PARALYZE:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_PARALYSIS, 0));
    break;
   case BTSPELLTYPE_DRAINLEVEL:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_LEVELDRAIN, 0));
    break;
   case BTSPELLTYPE_AGE:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), BTDice(0, 2), BTEXTRADAMAGE_AGED, 0));
    break;
   case BTSPELLTYPE_DAMAGEBYLEVEL:
    manifest.push_back(new BTAttackManifest(range, getEffectiveRange(), dice, BTEXTRADAMAGE_NONE, 1));
    break;
   case BTSPELLTYPE_CUREPOISON:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_POISONED));
    break;
   case BTSPELLTYPE_CUREINSANITY:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_INSANE));
    break;
   case BTSPELLTYPE_YOUTH:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_AGED));
    break;
   case BTSPELLTYPE_DISPOSSESS:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_POSSESSED));
    break;
   case BTSPELLTYPE_STONETOFLESH:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_STONED));
    break;
   case BTSPELLTYPE_CUREPARALYZE:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_PARALYZED));
    break;
   case BTSPELLTYPE_RESTORELEVELS:
    manifest.push_back(new BTCureStatusManifest(BTSTATUS_LEVELDRAIN));
    break;
   case BTSPELLTYPE_HEAL:
    manifest.push_back(new BTHealManifest(dice));
    break;
   case BTSPELLTYPE_PUSH:
    manifest.push_back(new BTPushManifest(getExtra()));
    break;
   case BTSPELLTYPE_REGENMANA:
    manifest.push_back(new BTRegenManaManifest(dice));
    break;
   case BTSPELLTYPE_BLOCKMAGIC:
    manifest.push_back(new BTTargetedManifest(type));
    break;
   case BTSPELLTYPE_SCRYSIGHT:
    manifest.push_back(new BTScrySightManifest());
    break;
   case BTSPELLTYPE_SUMMONMONSTER:
   case BTSPELLTYPE_SUMMONILLUSION:
   {
    manifest.push_back(new BTSummonManifest(type, getExtra()));
    break;
   }
   case BTSPELLTYPE_RESURRECT:
    manifest.push_back(new BTResurrectManifest());
    break;
   case BTSPELLTYPE_PHASEDOOR:
    manifest.push_back(new BTPhaseDoorManifest());
    break;
   case BTSPELLTYPE_DISPELLILLUSION:
   case BTSPELLTYPE_DISPELLMAGIC:
    manifest.push_back(new BTRangedManifest(type, range, getEffectiveRange()));
    break;
   case BTSPELLTYPE_SPELLBIND:
    manifest.push_back(new BTSpellBindManifest());
    break;
   case BTSPELLTYPE_REGENBARD:
   {
    BTDice amount(0, 4, getExtra());
    XMLVector<BTSkill*> &skillList = BTCore::getCore()->getSkillList();
    for (int which = 0; which < skillList.size(); ++which)
    {
     if ((skillList[which]->special == BTSKILLSPECIAL_SONG) && (skillList[which]->limited))
     {
      manifest.push_back(new BTRegenSkillManifest(which, amount));
      break;
     }
    }
    break;
   }
   default:
    break;
  }
 }
}

int BTSpellListCompare::Compare(const BTSpell &a, const BTSpell &b) const
{
 int ans = a.getCaster() - b.getCaster();
 if (0 == ans)
 {
  ans = a.getLevel() - b.getLevel();
  if (0 == ans)
   ans = strcmp(a.getName().c_str(), b.getName().c_str());
 }
 return ans;
}

