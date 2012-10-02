/*-------------------------------------------------------------------------*\
  <spell.C> -- Spell implementation file

  Date      Programmer  Description
  11/09/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "spell.h"
#include "game.h"

BTSpell::BTSpell(BinaryReadFile &f)
{
 IShort num;
 IUByte tmp[30];

 f.readUByteArray(29, tmp);
 tmp[29] = 0;
 name = new char[strlen((char *)tmp) + 1];
 strcpy(name, (char *)tmp);
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

BTSpell::BTSpell()
{
 name = new char[1];
 name[0] = 0;
 code = new char[1];
 code[0] = 0;
 effect = new char[1];
 effect[0] = 0;
}

BTSpell::~BTSpell()
{
 if (name)
  delete [] name;
 if (code)
  delete [] code;
 if (effect)
  delete [] effect;
}

const char *BTSpell::getName() const
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

const BTDice &BTSpell::getDice() const
{
 return dice;
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

int BTSpell::getExtra() const
{
 return extra;
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

int BTSpell::getType() const
{
 return type;
}

void BTSpell::write(BinaryWriteFile &f)
{
 IUByte unknown = 0x00;
 IShort num;
 char tmp[29];

 strncpy(tmp, name, 29);
 f.writeUByteArray(29, (IUByte *)tmp);
 strncpy(tmp, code, 5);
 f.writeUByteArray(5, (IUByte *)tmp);
 num = caster;
 f.writeShort(num);
 f.writeShort(level);
 f.writeShort(sp);
 f.writeShort(range);
 f.writeShort(effectiveRange);
 num = type;
 f.writeShort(num);
 num = area;
 f.writeShort(num);
 dice.write(f);
 f.writeUByte(unknown);
 num = duration;
 f.writeShort(num);
 num = extra;
 f.writeShort(num);
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
   break;
  case BTAREAEFFECT_FOE:
   text += " ";
   if (BTTARGET_PARTY == group)
   {
    text += party[target]->name;
   }
   text += ".";
  case BTAREAEFFECT_NONE:
  default:
   break;
 }
 d.drawMessage(text.c_str(), game->getDelay());
 BTBaseEffect *effect = NULL;
 switch(type)
 {
  case BTSPELLTYPE_RESURRECT:
   if (BTTARGET_PARTY == group)
   {
    if (BTTARGET_INDIVIDUAL == target)
    {
     effect = new BTResurrectEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target);
    }
    else
    {
     if (!party[target]->isAlive())
     {
      text = party[target]->name;
      text += " rises from the dead!";
      party[target]->status.clear(BTSTATUS_DEAD);
      party[target]->hp = 1;
      d.drawMessage(text.c_str(), game->getDelay());
      d.drawStats();
     }
    }
   }
   break;
  case BTSPELLTYPE_SUMMONMONSTER:
  case BTSPELLTYPE_SUMMONILLUSION:
  {
   if (party.size() >= BT_PARTYSIZE)
   {
    BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
    text = "No room in your party. ";
    text += monsterList[extra].getName();
    text += " cannot join!";
    d.drawMessage(text.c_str(), game->getDelay());
   }
   else
   {
    BTPc *pc = new BTPc(extra, ((type == BTSPELLTYPE_SUMMONMONSTER) ? BTJOB_MONSTER : BTJOB_ILLUSION));
    party.add(d, pc);
    d.drawStats();
    if ((BTTIME_PERMANENT != expire) && (BTTIME_CONTINUOUS != expire))
    {
     if (type == BTSPELLTYPE_SUMMONMONSTER)
      effect = new BTSummonMonsterEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, BTTARGET_PARTY, party.size() - 1);
     else
      effect = new BTSummonIllusionEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, BTTARGET_PARTY, party.size() - 1);
    }
   }
   break;
  }
  case BTSPELLTYPE_PHASEDOOR:
  {
   int x = game->getX();
   int y = game->getY();
   int f = game->getFacing();
   for (int i = 0; i < 4; ++i)
   {
    int testX = (x + (Psuedo3D::changeXY[f][0] * i) + game->getMap()->getXSize()) % game->getMap()->getXSize();
    int testY = (y + (Psuedo3D::changeXY[f][1] * i) + game->getMap()->getYSize()) % game->getMap()->getYSize();
    int str = game->testWallStrength(testX, testY, f);
    if (1 == str)
    {
     effect = new BTPhaseDoorEffect(type, BTTIME_MAP, BTTARGET_NOSINGER, BTMUSICID_NONE, testX, testY, f);
     break;
    }
    else if (2 == str)
     break;
   }
   break;
  }
  case BTSPELLTYPE_LIGHT:
  case BTSPELLTYPE_DOORDETECT:
  case BTSPELLTYPE_TRAPDESTROY:
  case BTSPELLTYPE_COMPASS:
  case BTSPELLTYPE_BLOCKENCOUNTERS:
   effect = new BTBaseEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE);
   break;
  case BTSPELLTYPE_DAMAGE:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, dice, BTEXTRADAMAGE_NONE);
   break;
  case BTSPELLTYPE_KILL:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_CRITICALHIT);
   break;
  case BTSPELLTYPE_POISON:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_POSION);
   break;
  case BTSPELLTYPE_CAUSEINSANITY:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_INSANITY);
   break;
  case BTSPELLTYPE_POSSESS:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_POSSESSION);
   break;
  case BTSPELLTYPE_FLESHTOSTONE:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_STONED);
   break;
  case BTSPELLTYPE_PARALYZE:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_PARALYSIS);
   break;
  case BTSPELLTYPE_DRAINLEVEL:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_LEVELDRAIN);
   break;
  case BTSPELLTYPE_AGE:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(0, 2), BTEXTRADAMAGE_AGED);
   break;
  case BTSPELLTYPE_DAMAGEBYLEVEL:
   effect = new BTAttackEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target, BTDice(dice.getNumber() * casterLevel, dice.getType(), dice.getModifier()), BTEXTRADAMAGE_NONE);
   break;
  case BTSPELLTYPE_CUREPOISON:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_POISONED);
   break;
  case BTSPELLTYPE_CUREINSANITY:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_INSANE);
   break;
  case BTSPELLTYPE_YOUTH:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_AGED);
   break;
  case BTSPELLTYPE_DISPOSSESS:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_POSSESSED);
   break;
  case BTSPELLTYPE_STONETOFLESH:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_STONED);
   break;
  case BTSPELLTYPE_CUREPARALYZE:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_PARALYZED);
   break;
  case BTSPELLTYPE_RESTORELEVELS:
   effect = new BTCureStatusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, BTSTATUS_LEVELDRAIN);
   break;
  case BTSPELLTYPE_HEAL:
   effect = new BTHealEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, dice);
   break;
  case BTSPELLTYPE_DISPELLILLUSION:
   effect = new BTDispellIllusionEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, range, getEffectiveRange(), distance, group, target);
   break;
  case BTSPELLTYPE_ARMORBONUS:
   effect = new BTArmorBonusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, getExtra());
   break;
  case BTSPELLTYPE_HITBONUS:
   effect = new BTHitBonusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, getExtra());
   break;
  case BTSPELLTYPE_REGENBARD:
  {
   BTDice amount(0, 4, getExtra());
   XMLVector<BTSkill*> &skillList = BTGame::getGame()->getSkillList();
   for (int which = 0; which < skillList.size(); ++which)
   {
    if ((skillList[which]->special == BTSKILLSPECIAL_SONG) && (skillList[which]->limited))
    {
     effect = new BTRegenSkillEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, which, amount);
     break;
    }
   }
   break;
  }
  case BTSPELLTYPE_PUSH:
   effect = new BTPushEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, getExtra());
   break;
  case BTSPELLTYPE_ATTACKRATEBONUS:
   effect = new BTAttackRateBonusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, getExtra());
   break;
  case BTSPELLTYPE_REGENMANA:
   effect = new BTRegenManaEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, dice);
   break;
  case BTSPELLTYPE_SAVEBONUS:
   effect = new BTSaveBonusEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target, getExtra());
   break;
  case BTSPELLTYPE_BLOCKMAGIC:
   effect = new BTTargetedEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE, group, target);
   break;
  case BTSPELLTYPE_SCRYSIGHT:
   effect = new BTScrySightEffect(type, expire, BTTARGET_NOSINGER, BTMUSICID_NONE);
   break;
  default:
   break;
 }
 if (effect)
 {
  try
  {
   killed = effect->apply(d, combat);
   game->addEffect(effect);
  }
  catch (const BTAllResistException &e)
  {
   delete effect;
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
 s->add("caster", &caster, NULL, &BTGame::getGame()->getSkillList());
 s->add("level", &level);
 s->add("sp", &sp);
 s->add("range", &range);
 s->add("effectiveRange", &effectiveRange);
 s->add("type", &type, NULL, &spellTypeLookup);
 s->add("area", &area, NULL, &areaLookup);
 s->add("dice", &dice);
 s->add("duration", &duration, NULL, &durationLookup);
 s->add("extra", &extra);
 s->add("effect", &effect);
}

void BTSpell::readXML(const char *filename, XMLVector<BTSpell*> &spell)
{
 XMLSerializer parser;
 parser.add("spell", &spell, &BTSpell::create);
 parser.parse(filename, true);
}

void BTSpell::writeXML(const char *filename, XMLVector<BTSpell*> &spell)
{
 XMLSerializer parser;
 parser.add("spell", &spell, &BTSpell::create);
 parser.write(filename, true);
}

int BTSpellListCompare::Compare(const BTSpell &a, const BTSpell &b) const
{
 int ans = a.getCaster() - b.getCaster();
 if (0 == ans)
 {
  ans = a.getLevel() - b.getLevel();
 }
 return ans;
}

