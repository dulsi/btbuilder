/*-------------------------------------------------------------------------*\
  <map.C> -- Map implementation file

  Date      Programmer  Description
  11/12/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "btconst.h"
#include "screenset.h"
#include "map.h"
#include "game.h"
#include "pc.h"

BTMapSquare::BTMapSquare()
 : special(-1)
{
 for (int i = 0; i < BT_DIRECTIONS; ++i)
 {
  wallInfo[i] = 0;
 }
}

IShort BTMapSquare::getWall(IShort dir) const
{
 return wallInfo[dir];
}

IShort BTMapSquare::getSpecial() const
{
 return special;
}

void BTMapSquare::read(BinaryReadFile &f)
{
 IUByte tmp;

 f.readUByte(tmp);
 for (int i = 0; i < BT_DIRECTIONS; ++i)
  wallInfo[i] = ((tmp >> (i * 2)) & 0x03);
 f.readUByte(tmp); // Unknown
 f.readShort(special);
}

void BTMapSquare::setWall(IShort dir, IShort wall)
{
 wallInfo[dir] = wall;
}

void BTMapSquare::setSpecial(IShort s)
{
 special = s;
}

void BTMapSquare::serialize(ObjectSerializer* s)
{
 s->add("northgfx", &wallInfo[BTDIRECTION_NORTH]);
 s->add("eastgfx", &wallInfo[BTDIRECTION_EAST]);
 s->add("southgfx", &wallInfo[BTDIRECTION_SOUTH]);
 s->add("westgfx", &wallInfo[BTDIRECTION_WEST]);
 s->add("special", &special);
}

void BTMapSquare::write(BinaryWriteFile &f)
{
 IUByte tmp;

 tmp = 0;
 for (int i = BT_DIRECTIONS; i > 0; --i)
 {
  if (wallInfo[i - 1] > 3)
   throw FileException("Wall type not supported.");
  tmp = tmp << 2;
  tmp += wallInfo[i - 1];
 }
 f.writeUByte(tmp);
 tmp = 0;
 f.writeUByte(tmp); // Unknown
 f.writeShort(special);
}

BTSpecialBody::BTSpecialBody(const BTSpecialBody &copy)
{
 for (int i = 0; i < copy.ops.size(); ++i)
 {
  ops.push_back(copy.ops[i]->clone());
 }
}

void BTSpecialBody::eraseOperation(BTSpecialOperation *op)
{
 for (int i = 0; i < ops.size(); ++i)
 {
  if (ops[i] == op)
  {
   ops.erase(ops.begin() + i);
   return;
  }
 }
}

int BTSpecialBody::findLabel(const std::string &l) const
{
 for (int i = 0; i < ops.size(); ++i)
 {
  BTSpecialCommand *cmd = dynamic_cast<BTSpecialCommand*>(ops[i]);
  if ((cmd) && (cmd->getType() == BTSPECIALCOMMAND_LABEL) && (l == cmd->getText()))
   return i;
 }
 return ops.size();
}

BTSpecialOperation *BTSpecialBody::getOperation(int line)
{
 if (line < ops.size())
  return ops[line];
 return NULL;
}

void BTSpecialBody::insertOperation(BTSpecialOperation *before, BTSpecialOperation *op)
{
 for (int i = 0; i < ops.size(); ++i)
 {
  if (ops[i] == before)
  {
   ops.insert(ops.begin() + i, op);
   return;
  }
 }
}

void BTSpecialBody::replaceOperation(BTSpecialOperation *opOld, BTSpecialOperation *opNew)
{
 for (int i = 0; i < ops.size(); ++i)
 {
  if (ops[i] == opOld)
  {
   ops[i] = opNew;
   delete opOld;
   return;
  }
 }
}

BTSpecialOperation *BTSpecialBody::clone() const
{
 return new BTSpecialBody(*this);
}

IBool BTSpecialBody::isNothing() const
{
 int i = 0;
 while (i < ops.size())
 {
  if (!ops[i]->isNothing())
   return false;
  ++i;
 }
 return true;
}

void BTSpecialBody::moveFrom(BTSpecialBody *body)
{
 for (int i = body->numOfOperations(false); i > 0; --i)
 {
  addOperation(body->getOperation(0));
  body->ops.erase(body->ops.begin(), false);
 }
}

int BTSpecialBody::numOfOperations(bool recursive) const
{
 if (recursive)
 {
  int count = 0;
  for (int i = 0; i < ops.size(); ++i)
  {
   ++count;
   BTSpecialBody *subBody = dynamic_cast<BTSpecialBody*>(ops[i]);
   if (subBody)
   {
    count += subBody->numOfOperations(recursive);
   }
   else
   {
    BTSpecialConditional *conditional = dynamic_cast<BTSpecialConditional*>(ops[i]);
    if (conditional)
    {
     count += conditional->getThenClause()->numOfOperations(recursive);
     if (conditional->getElseClause()->numOfOperations(false) > 0)
     {
      count += 1 + conditional->getElseClause()->numOfOperations(recursive);
     }
    }
   }
  }
 }
 else
  return ops.size();
}

std::string BTSpecialBody::print() const
{
 return "body";
}

void BTSpecialBody::print(FILE *f, int indent) const
{
 int i = 0;
 while (i < ops.size())
 {
  ops[i]->print(f, indent + 1);
  ++i;
 }
}

void BTSpecialBody::run(BTDisplay &d) const
{
 runFromLine(d, 0);
}

void BTSpecialBody::runFromLine(BTDisplay &d, int line) const
{
 while (line < ops.size())
 {
  ops[line]->run(d);
  ++line;
 }
}

void BTSpecialBody::serialize(ObjectSerializer* s)
{
 s->add("conditional", typeid(BTSpecialConditional).name(), &ops, &BTSpecialConditional::create);
 s->add("command", typeid(BTSpecialCommand).name(), &ops, &BTSpecialCommand::create);
 s->add("body", typeid(BTSpecialBody).name(), &ops, &BTSpecialBody::create);
}

void BTSpecialBody::upgradeToLabel(BitField &labelNeeded)
{
 char tmp[27];
 for (int i = 0; i < ops.size(); ++i)
 {
  BTSpecialConditional *conditional = dynamic_cast<BTSpecialConditional*>(ops[i]);
  if (conditional)
  {
   conditional->getThenClause()->upgradeToLabel(labelNeeded);
   conditional->getElseClause()->upgradeToLabel(labelNeeded);
  }
  else
  {
   BTSpecialCommand *command = dynamic_cast<BTSpecialCommand*>(ops[i]);
   if ((command) && (command->getType() == BTSPECIALCOMMAND_GOTO))
   {
    labelNeeded.set(command->getNumber(0));
    snprintf(tmp, 26, "line number %d", command->getNumber(0));
    command->setText(tmp);
   }
  }
 }
}

BTSpecialCommand::BTSpecialCommand()
{
 type = 0;
 text = new char[1];
 text[0] = 0;
}

BTSpecialCommand::~BTSpecialCommand()
{
 if (text)
  delete [] text;
}

BTSpecialOperation *BTSpecialCommand::clone() const
{
 BTSpecialCommand *copy = new BTSpecialCommand(type);
 copy->setText(text);
 for (int i = 0; i < 3; ++i)
  copy->setNumber(i, number[i]);
 return copy;
}

IShort BTSpecialCommand::getType() const
{
 return type;
}

std::string BTSpecialCommand::getText() const
{
 return (text ? text : "");
}

IUShort BTSpecialCommand::getNumber(int indx) const
{
 return number[indx];
}

IBool BTSpecialCommand::isNothing() const
{
 return (0 == getType());
}

std::string BTSpecialCommand::print() const
{
 char *dollarSign;
 char *start;
 long len;
 int count;
 std::string answer;

 count = 0;
 start = specialCommands[type];
 while (dollarSign = strchr(start, '$'))
 {
  len = (long)dollarSign - (long)start;
  answer += std::string(start, len);
  switch (dollarSign[1])
  {
   case 'S':
    answer += BTCore::getCore()->getMap()->getSpecial(number[count++])->getName();
    break;
   case 'I':
    answer += BTCore::getCore()->getItemList()[number[count++]].getName();
    break;
   case 'A':
   case 'M':
    answer += BTCore::getCore()->getMonsterList()[number[count++]].getName().c_str();
    break;
   case 'X':
    answer += BTCore::getCore()->getSpellList()[number[count++]].getName();
    break;
   case 'L':
   {
    char s[50];
    snprintf(s, 50, "X:%d Y:%d", number[count], number[count + 1]);
    count += 2;
    answer += s;
    break;
   }
   case 'T':
    answer += extraDamage[number[count++]];
    break;
   case 'C':
    answer += BTCore::getCore()->getJobList()[number[count++]]->name;
    break;
   case 'R':
    answer += BTCore::getCore()->getRaceList()[number[count++]]->name;
    break;
   case 'D':
    answer += directions[number[count++]];
    break;
   case '#':
   case 'P':
   case 'G':
   case 'F':
   case '!':
   case 'J':
   {
    char s[50];
    snprintf(s, 50, "%d", number[count++]);
    answer += s;
    break;
   }
   case '$':
   case 'N':
   default:
    answer += text;
    break;
  }
  start = dollarSign + 2;
 }
 answer += start;
 return answer;
}

void BTSpecialCommand::print(FILE *f, int indent) const
{
 for (int k = 0; k < indent; ++k)
  fprintf(f, "    ");
 fprintf(f, "%s\n", print().c_str());
}

void BTSpecialCommand::read(BinaryReadFile &f)
{
 char tmp[27];
 f.readShort(type);
 f.readUByteArray(26, (IUByte *)tmp);
 tmp[26] = 0;
 if (text)
  delete [] text;
 text = new char[strlen(tmp) + 1];
 strcpy(text, tmp);
 f.readShortArray(3, (IShort *)number);
}

void BTSpecialCommand::run(BTDisplay &d) const
{
 BTGame *game = BTGame::getGame();
 switch (type)
 {
  case BTSPECIALCOMMAND_STOP:
   throw BTSpecialStop();
   break;
  case BTSPECIALCOMMAND_SPIN:
   game->setFacing(BTDice(1, 4, 0).roll() % 4);
   break;
  case BTSPECIALCOMMAND_DARKNESS:
  {
   BitField newFlag;
   newFlag.set(BTSPECIALFLAG_DARKNESS);
   game->addFlags(d, newFlag);
   break;
  }
  case BTSPECIALCOMMAND_SILENCE:
  {
   d.drawText("The sound of silence...");
   BitField newFlag;
   newFlag.set(BTSPECIALFLAG_SILENCE);
   game->addFlags(d, newFlag);
   break;
  }
  case BTSPECIALCOMMAND_ANTIMAGIC:
  {
   BitField newFlag;
   newFlag.set(BTSPECIALFLAG_ANTIMAGIC);
   game->addFlags(d, newFlag);
   break;
  }
  case BTSPECIALCOMMAND_GETINPUT:
   game->setLastInput(d.readString("", 13, ""));
   break;
  case BTSPECIALCOMMAND_SETLOCALFLAG:
   game->setLocalFlag(number[0], true);
   break;
  case BTSPECIALCOMMAND_CLEARLOCALFLAG:
   game->setLocalFlag(number[0], false);
   break;
  case BTSPECIALCOMMAND_TAKEITEM:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->takeItem(number[0]))
     break;
   }
   break;
  }
  case BTSPECIALCOMMAND_GIVEITEM:
  {
   BTParty &party = game->getParty();
   party.giveItem(number[0], d);
   break;
  }
  case BTSPECIALCOMMAND_SELLITEM:
   d.drawText("");
   d.drawText("Pick a party member:");
   while (true)
   {
    char key = d.readChar();
    if (27 == key)
     break;
    else if (('1' <= key) && ('9' >= key))
    {
     XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
     BTFactory<BTItem> &itemList = BTGame::getGame()->getItemList();
     int n =  key - '1';
     if (n < party.size())
     {
      char tmp[100];
      if (party[n]->isAlive())
      {
       d.drawText(party[n]->name);
       snprintf(tmp, 100, "%s costs %d gold coins.", itemList[number[0]].getName().c_str(), number[1]);
       d.drawText(tmp);
       d.drawText("Wilt thou pay?");
       d.drawText("Yes, or");
       d.drawText("No");
       while (true)
       {
        key = d.readChar();
        if ((27 == key) || ('Y' == key) || ('y' == key) || ('N' == key) || ('n' == key))
         break;
       }
       if (('Y' == key) || ('y' == key))
       {
        if (party[n]->getGold() < number[1])
         snprintf(tmp, 100, "%s does not have enough gold!", party[n]->name);
        else if (party[n]->isEquipmentFull())
        {
         snprintf(tmp, 100, "%s has no room for %s!", party[n]->name, itemList[number[0]].getName().c_str());
         d.drawText(tmp);
         snprintf(tmp, 100, "%s does not have room to carry it!", party[n]->name);
        }
        else
        {
         party[n]->takeGold(number[1]);
         party[n]->giveItem(number[0], true, itemList[number[0]].getTimesUsable());
         snprintf(tmp, 100, "%s gets %s.", party[n]->name, itemList[number[0]].getName().c_str());
        }
        d.drawText(tmp);
       }
      }
      else
      {
       snprintf(tmp, 100, "%s is dead!", party[n]->name);
       d.drawText(tmp);
      }
      break;
     }
    }
   }
   break;
  case BTSPECIALCOMMAND_CASTSPELL:
  {
   BTFactory<BTSpell, BTSpell1> &spellList = BTGame::getGame()->getSpellList();
   if (0 == number[1])
   {
    spellList[number[0]].activate(d, "", true, NULL, game->getMap()->getLevel(), 0, BTTARGET_PARTY, ((BTAREAEFFECT_FOE == spellList[number[0]].getArea()) ? 0 : BTTARGET_INDIVIDUAL));
   }
   else
   {
    char tmp[100];
    snprintf(tmp, 100, "Who desires %s?", spellList[number[0]].getName().c_str());
    d.drawText(tmp);
    d.drawText("");
    d.drawText("Pick a party member:");
    while (true)
    {
     char key = d.readChar();
     if (27 == key)
      break;
     else if (('1' <= key) && ('9' >= key))
     {
      XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
      int n =  key - '1';
      if (n < party.size())
      {
       d.drawText(party[n]->name);
       snprintf(tmp, 100, "%s costs %d gold coins.", spellList[number[0]].getName().c_str(), number[1]);
       d.drawText(tmp);
       d.drawText("Wilt thou pay?");
       d.drawText("Yes, or");
       d.drawText("No");
       while (true)
       {
        key = d.readChar();
        if ((27 == key) || ('Y' == key) || ('y' == key) || ('N' == key) || ('n' == key))
         break;
       }
       if (('Y' == key) || ('y' == key))
       {
        if (party[n]->getGold() < number[1])
        {
         snprintf(tmp, 100, "%s does not have enough gold!", party[n]->name);
         d.drawText(tmp);
        }
        else
        {
         party[n]->takeGold(number[1]);
         spellList[number[0]].activate(d, "", true, NULL, game->getMap()->getLevel(), 0, BTTARGET_PARTY, ((BTAREAEFFECT_FOE == spellList[number[0]].getArea()) ? n : BTTARGET_INDIVIDUAL));
        }
       }
       break;
      }
     }
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_PRINT:
   d.drawText(text);
   break;
  case BTSPECIALCOMMAND_REGENERATESPELLS:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->sp < party[who]->maxSp)
    {
     if (party[who]->sp + number[0] < party[who]->maxSp)
      party[who]->sp += number[0];
     else
      party[who]->sp = party[who]->maxSp;
    }
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_TAKESPELLS:
  {
   XMLVector<BTPc*> &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->sp > 0)
    {
     if (party[who]->sp < number[0])
      party[who]->sp = 0;
     else
      party[who]->sp -= number[0];
    }
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_HEALHITPOINTS:
  {
   BTParty &party = game->getParty();
   for (int who = 0; who < party.size(); ++who)
   {
    // BTCS heals even if dead.  Not sure if we want to mimic that
    if (party[who]->status.isSet(BTSTATUS_DEAD))
    {
     break;
    }
    party[who]->giveHP(number[0]);
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_DAMAGEHITPOINTS:
  {
   BTParty &party = game->getParty();
   char tmp[100];
   for (int who = 0; who < party.size(); ++who)
   {
    if (party[who]->status.isSet(BTSTATUS_DEAD))
    {
     break;
    }
    bool saved = party[who]->savingThrow(BTSAVE_DIFFICULTY);
    if (saved)
    {
     snprintf(tmp, 100, "%s saves!", party[who]->name);
     d.drawText(tmp);
    }
    // Still need to handle special damage like poison
    if (party[who]->takeHP((saved ? number[0] >> 1 : number[0])))
    {
     snprintf(tmp, 100, "%s dies!", party[who]->name);
     d.drawText(tmp);
    }
   }
   if (party.checkDead(d))
    throw BTPartyDead();
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_REGENERATESONGS:
  {
   XMLVector<BTPc*> &party = game->getParty();
   BTSkillList &skillList = game->getSkillList();
   for (int who = 0; who < party.size(); ++who)
   {
    for (int which = 0; which < skillList.size(); ++which)
    {
     if ((skillList[which]->special == BTSKILLSPECIAL_SONG) && (skillList[which]->limited))
      party[who]->giveSkillUse(which, number[0]);
    }
   }
   d.drawStats();
   break;
  }
  case BTSPECIALCOMMAND_BACKONE:
   throw BTSpecialBack();
   break;
  case BTSPECIALCOMMAND_FORWARDONE:
   throw BTSpecialForward();
   break;
  case BTSPECIALCOMMAND_TELEPORT:
   throw BTSpecialTeleport(text, number[0], -number[1] - 1, number[2], false);
   break;
  case BTSPECIALCOMMAND_GUILD:
  {
   BTScreenSet b;
   b.open("data/guild.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_REVIEW:
  {
   BTScreenSet b;
   b.open("data/review.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_SHOP:
  {
   BTScreenSet b;
   b.open("data/shop.xml");
   b.run(d);
   break;
  }
  case BTSPECIALCOMMAND_TRAP:
  {
   if (game->hasEffectOfType(BTSPELLTYPE_TRAPDESTROY))
   {
    d.drawText("A trap is magically disabled!");
    throw BTSpecialStop();
   }
   else
   {
    BTParty &party = BTGame::getGame()->getParty();
    BTSkillList &skillList = game->getSkillList();
    for (int i = 0; i < skillList.size(); ++i)
    {
     if (skillList[i]->special == BTSKILLSPECIAL_DISARM)
     {
      for (int k = 0; k < party.size(); ++k)
      {
       if (party[k]->useSkill(i))
       {
        char tmp[100];
        snprintf(tmp, 100, "%s finds and disarms a trap!", party[k]->name);
        d.drawText(tmp);
        throw BTSpecialStop();
       }
      }
     }
    }
   }
  }
  case BTSPECIALCOMMAND_DRAWPICTURE:
   d.drawImage(number[0]);
   break;
  case BTSPECIALCOMMAND_CLEARSPECIALAT:
  {
   BTMap *map = game->getMap();
   game->getMap()->setSpecial(number[0], map->getYSize() - 1 - number[1], BTSPECIAL_NONE);
   break;
  }
  case BTSPECIALCOMMAND_SETSPECIALAT:
  {
   BTMap *map = game->getMap();
   map->setSpecial(number[1], map->getYSize() - 1 - number[2], number[0]);
   break;
  }
  case BTSPECIALCOMMAND_PRINTLABEL:
   d.drawLabel(text);
   break;
  case BTSPECIALCOMMAND_MONSTERJOIN:
  {
   BTFactory<BTMonster> &monsterList = BTGame::getGame()->getMonsterList();
   char tmp[100];
   snprintf(tmp, 100, "%s asks to join the party! Do you say", monsterList[number[0]].getName().c_str());
   d.drawText(tmp);
   d.drawText("Yes, or");
   d.drawText("No");
   char key;
   while (true)
   {
    key = d.readChar();
    if ((27 == key) || ('Y' == key) || ('y' == key) || ('N' == key) || ('n' == key))
     break;
   }
   if (('Y' == key) || ('y' == key))
   {
    BTParty &party = BTGame::getGame()->getParty();
    if (party.size() >= BT_PARTYSIZE)
    {
     snprintf(tmp, 100, "No room in your party. %s cannot join!", monsterList[number[0]].getName().c_str());
     d.drawText(tmp);
    }
    else
    {
     BTPc *pc = new BTPc(number[0], BTJOB_MONSTER); // Illusions become monster for some reason
     party.add(d, pc);
     d.drawStats();
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_ALLMONSTERSLEAVE:
  {
   BTParty &party = BTGame::getGame()->getParty();
   for (int i = party.size(); i > 0; )
   {
    --i;
    if (party[i]->job == BTJOB_MONSTER)
    {
     char tmp[100];
     snprintf(tmp, 100, "%s leaves your party.", party[i]->name);
     d.drawText(tmp);
     party.remove(i, d);
     d.drawStats();
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_ALLMONSTERSHOSTILE:
  {
   BTParty &party = BTGame::getGame()->getParty();
   for (int i = party.size(); i > 0; )
   {
    --i;
    if (party[i]->job == BTJOB_MONSTER)
    {
     party[i]->status.set(BTSTATUS_INSANE);
     d.drawStats();
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_PLAYSOUND:
  {
   char tmp[100];
   snprintf(tmp, 100, "sound/sound%d.wav", number[0]);
   d.playSound(tmp, true);
   break;
  }
  case BTSPECIALCOMMAND_SETCOUNTER:
   game->setCounter(number[0]);
   break;
  case BTSPECIALCOMMAND_ADDCOUNTER:
   game->setCounter(game->getCounter() + number[0]);
   break;
  case BTSPECIALCOMMAND_SETENCOUNTER:
   game->getCombat().addEncounter(number[0]);
   break;
  case BTSPECIALCOMMAND_SETNUMENCOUNTER:
   game->getCombat().addEncounter(number[1], number[0]);
   break;
  case BTSPECIALCOMMAND_BEGINCOMBAT:
   game->getCombat().run(d);
   break;
  case BTSPECIALCOMMAND_CHEST:
  {
   game->getChest().setup(text, number[1], BTDice(0, 6, number[2]), number[0]);
   BTScreenSet b;
   b.open("data/chest.xml");
   if (!game->getChest().isTrapped())
    b.setEffect(BTSPELLTYPE_TRAPDESTROY);
   b.run(d);
   game->getChest().clear();
   break;
  }
  case BTSPECIALCOMMAND_SETGLOBALFLAG:
   game->setGlobalFlag(number[0], true);
   break;
  case BTSPECIALCOMMAND_CLEARGLOBALFLAG:
   game->setGlobalFlag(number[0], false);
   break;
  case BTSPECIALCOMMAND_GIVEGOLD:
  {
   // Loses extra gold like BTCS.  Should fix.
   XMLVector<BTPc*> &party = game->getParty();
   int gold = number[0] / party.size();
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->giveGold(gold);
   }
   break;
  }
  case BTSPECIALCOMMAND_GIVEXP:
  {
   XMLVector<BTPc*> &party = game->getParty();
   int xp = number[0] / party.size();
   for (int i = 0; i < party.size(); ++i)
   {
    party[i]->giveXP(xp);
   }
   break;
  }
  case BTSPECIALCOMMAND_PRESSANYKEY:
   d.drawText("(Press any key)");
   d.readChar();
   break;
  case BTSPECIALCOMMAND_MONSTERLEAVE:
  {
   BTParty &party = BTGame::getGame()->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if ((party[i]->job == BTJOB_MONSTER) && (party[i]->monster == number[0]) && (0 == strcmp(party[i]->name, BTGame::getGame()->getMonsterList()[number[0]].getName().c_str())))
    {
     char tmp[100];
     snprintf(tmp, 100, "%s leaves your party.", party[i]->name);
     d.drawText(tmp);
     party.remove(i, d);
     d.drawStats();
     break;
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_MONSTERHOSTILE:
  {
   BTParty &party = BTGame::getGame()->getParty();
   for (int i = 0; i < party.size(); ++i)
   {
    if ((party[i]->job == BTJOB_MONSTER) && (party[i]->monster == number[0]) && (0 == strcmp(party[i]->name, BTGame::getGame()->getMonsterList()[number[0]].getName().c_str())))
    {
     party[i]->status.set(BTSTATUS_INSANE);
     d.drawStats();
     break;
    }
   }
   break;
  }
  case BTSPECIALCOMMAND_SETDIRECTION:
   game->setFacing(number[0]);
   break;
  case BTSPECIALCOMMAND_CLEARSPECIAL:
   game->getMap()->setSpecial(game->getX(), game->getY(), BTSPECIAL_NONE);
   break;
  case BTSPECIALCOMMAND_SETSPECIAL:
   game->getMap()->setSpecial(game->getX(), game->getY(), number[0]);
   break;
  case BTSPECIALCOMMAND_RANDOMENCOUNTER:
   game->getMap()->generateRandomEncounter(d, 1);
   break;
  case BTSPECIALCOMMAND_CLEARTEXT:
   d.clearText();
   break;
  case BTSPECIALCOMMAND_TAKEGOLD:
  {
   XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
   int left = number[0];
   for (int i = 0; i < party.size(); ++i)
   {
    left -= party[i]->takeGold(left);
    if (left <= 0)
     break;
   }
   break;
  }
  case BTSPECIALCOMMAND_DRAWFULLPICTURE:
   d.drawFullScreen(text, 0);
   d.refresh();
   break;
  case BTSPECIALCOMMAND_SETTIMESPECIAL:
   game->setTimedSpecial(number[0], game->getExpiration(number[1] * BTTIME_MINUTE));
   break;
  case BTSPECIALCOMMAND_CLEARTIMESPECIAL:
   game->clearTimedSpecial();
   break;
  case BTSPECIALCOMMAND_GOTO:
   throw BTSpecialGoto(text);
   break;
  case BTSPECIALCOMMAND_SUBTRACTCOUNTER:
   game->setCounter(game->getCounter() - number[0]);
   break;
  case BTSPECIALCOMMAND_TELEPORTRELATIVE:
   throw BTSpecialTeleport(text, number[0] + game->getX(), game->getY() - 1 - number[1], number[2], false);
   break;
  case BTSPECIALCOMMAND_TELEACTIVATE:
   throw BTSpecialTeleport(text, number[0], -number[1] - 1, number[2], true);
   break;
  default:
   break;
 }
}

void BTSpecialCommand::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
 s->add("text", &text);
 for (int i = 0; i < 3; ++i)
 {
  std::vector<XMLAttribute> *attrib = new std::vector<XMLAttribute>;
  char tmp[10];
  snprintf(tmp, 10, "%d", i + 1);
  attrib->push_back(XMLAttribute("index", tmp));
  s->add("number", &number[i], attrib);
 }
}

void BTSpecialCommand::setText(const std::string &t)
{
 text = new char[t.length() + 1];
 strcpy(text, t.c_str());
}

void BTSpecialCommand::setNumber(int indx, IUShort value)
{
 number[indx] = value;
}

void BTSpecialCommand::write(BinaryWriteFile &f)
{
 char tmp[26];
 size_t len = strlen(text);
 if (len > 25)
  throw FileException("Text is too long.");
 f.writeShort(type);
 strcpy(tmp, text);
 memset(tmp + len, 0, 26 - len);
 f.writeUByteArray(26, (IUByte *)tmp);
 f.writeShortArray(3, (IShort *)number);
}

BTSpecialCommand BTSpecialCommand::Guild(BTSPECIALCOMMAND_GUILD);

BTSpecialConditional::BTSpecialConditional()
{
 type = -1;
 text = new char[1];
 text[0] = 0;
}

BTSpecialConditional::BTSpecialConditional(const BTSpecialConditional &copy)
 : type(copy.type), number(copy.number), thenClause(copy.thenClause), elseClause(copy.elseClause)
{
 text = new char[strlen(copy.text) + 1];
 strcpy(text, copy.text);
}

BTSpecialConditional::BTSpecialConditional(IShort t, const char *txt, IShort num)
: type(t), number(num)
{
 text = new char[strlen(txt) + 1];
 strcpy(text, txt);
}

BTSpecialConditional::~BTSpecialConditional()
{
 if (text)
  delete [] text;
}

BTSpecialOperation *BTSpecialConditional::clone() const
{
 return new BTSpecialConditional(*this);
}

IShort BTSpecialConditional::getType() const
{
 return type;
}

IBool BTSpecialConditional::isNothing() const
{
 return ((-1 == type) && (thenClause.isNothing()));
}

std::string BTSpecialConditional::print() const
{
 char *dollarSign;
 long len;
 std::string answer;

 answer += "IF   ";
 dollarSign = strchr(conditionalCommands[type], '$');
 if (dollarSign)
 {
  len = (long)dollarSign - (long)conditionalCommands[type];
  answer += std::string(conditionalCommands[type], len);
  switch (dollarSign[1])
  {
   case 'I':
    answer += BTCore::getCore()->getItemList()[number].getName();
    break;
   case 'A':
    answer += BTCore::getCore()->getMonsterList()[number].getName().c_str();
    break;
   case 'C':
    answer += BTCore::getCore()->getJobList()[number]->name;
    break;
   case 'R':
    answer += BTCore::getCore()->getRaceList()[number]->name;
    break;
   case 'D':
    answer += directions[number];
    break;
   case '#':
   case 'G':
   case 'F':
   {
    char s[50];
    snprintf(s, 50, "%d", number);
    answer += s;
    break;
   }
   case 'E':
    answer += spellTypes[number];
    break;
   case '$':
   default:
    answer += text;
    break;
  }
  dollarSign += 2;
  answer += dollarSign;
 }
 else
 {
  answer += conditionalCommands[type];
 }
 return answer;
}

void BTSpecialConditional::print(FILE *f, int indent) const
{
 for (int k = 0; k < indent; ++k)
  fprintf(f, "    ");
 fprintf(f, "%s\n", print().c_str());
 for (int k = 0; k < indent; ++k)
  fprintf(f, "    ");
 fprintf(f, "THEN\n");
 thenClause.print(f, indent);
 for (int k = 0; k < indent; ++k)
  fprintf(f, "    ");
 fprintf(f, "ELSE\n");
 elseClause.print(f, indent);
}

void BTSpecialConditional::read(BinaryReadFile &f)
{
 char tmp[27];
 f.readShort(type);
 f.readUByteArray(26, (IUByte *)tmp);
 tmp[26] = 0;
 f.readShort(number);
 BTSpecialCommand *op = new BTSpecialCommand;
 op->read(f);
 thenClause.addOperation(op);
 op = new BTSpecialCommand;
 op->read(f);
 elseClause.addOperation(op);
}

void BTSpecialConditional::run(BTDisplay &d) const
{
 XMLVector<BTPc*> &party = BTGame::getGame()->getParty();
 bool truth = true;
 switch (type)
 {
  case BTCONDITION_ANYONEITEM:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
   {
    if (party[i]->hasItem(number))
    {
     truth = true;
     break;
    }
   }
   break;
  }
  case BTCONDITION_EVERYONEITEM:
  {
   for (int i = 0; i < party.size(); ++i)
   {
    if (!party[i]->hasItem(number))
    {
     truth = false;
     break;
    }
   }
   break;
  }
  case BTCONDITION_LASTINPUT:
   truth = (strcmp(BTGame::getGame()->getLastInput().c_str(), text) == 0);
   break;
  case BTCONDITION_ANSWERYES:
  {
   d.drawText(text);
   std::string ans = d.readString("", 3, "");
   truth = ((strcmp(ans.c_str(), "y") == 0) ||
    (strcmp(ans.c_str(), "yes") == 0));
   break;
  }
  case BTCONDITION_LOCALFLAGSET:
   truth = (true == BTGame::getGame()->getLocalFlag(number));
   break;
  case BTCONDITION_LOCALFLAGCLEAR:
   truth = (false == BTGame::getGame()->getLocalFlag(number));
   break;
  case BTCONDITION_GROUPFACING:
   truth = (BTGame::getGame()->getFacing() == number);
   break;
  case BTCONDITION_MONSTERINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if ((party[i]->job == BTJOB_MONSTER) && (party[i]->monster == number) && (0 == strcmp(party[i]->name, BTGame::getGame()->getMonsterList()[number].getName().c_str())))
     truth = true;
   break;
  }
  case BTCONDITION_JOBINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i]->race == number)
     truth = true;
   break;
  }
  case BTCONDITION_DAYTIME:
   truth = BTGame::getGame()->isDaytime();
   break;
  case BTCONDITION_COUNTERGREATER:
   truth = ((BTGame::getGame()->getCounter() > number) ? true : false);
   break;
  case BTCONDITION_COUNTEREQUAL:
   truth = ((BTGame::getGame()->getCounter() == number) ? true : false);
   break;
  case BTCONDITION_COMBATWON:
   truth = BTGame::getGame()->getCombat().isWinner();
   break;
  case BTCONDITION_GLOBALFLAGSET:
   truth = (true == BTGame::getGame()->getGlobalFlag(number));
   break;
  case BTCONDITION_GLOBALFLAGCLEAR:
   truth = (false == BTGame::getGame()->getGlobalFlag(number));
   break;
  case BTCONDITION_RANDOM:
  {
   int roll = BTDice(1, 100).roll();
   truth = (roll <= number);
   break;
  }
  case BTCONDITION_RACEINPARTY:
  {
   truth = false;
   for (int i = 0; i < party.size(); ++i)
    if (party[i]->race == number)
     truth = true;
   break;
  }
  case BTCONDITION_CANTAKEGOLD:
  {
   int gold = 0;
   truth = false;
   for (int i = 0; i < party.size(); ++i)
   {
    gold += party[i]->getGold();
    if (gold > number)
     truth = true;
   }
   break;
  }
  case BTCONDITION_EFFECTACTIVE:
  {
   truth = BTGame::getGame()->hasEffectOfType(number);
  }
  default:
   break;
 }
 if (truth)
  thenClause.run(d);
 else
  elseClause.run(d);
}

void BTSpecialConditional::setType(IShort val)
{
 type = val;
}

void BTSpecialConditional::serialize(ObjectSerializer* s)
{
 s->add("type", &type);
 s->add("text", &text);
 s->add("number", &number);
 s->add("then", &thenClause);
 s->add("else", &elseClause);
}

void BTSpecialConditional::write(BinaryWriteFile &f)
{
 char tmp[26];
 size_t len = strlen(text);
 if (len > 25)
  throw FileException("Text is too long.");
 if (thenClause.ops.size() > 1)
  throw FileException("Too many operations in then clause.");
 if (elseClause.ops.size() > 1)
  throw FileException("Too many operations in else clause.");
 BTSpecialCommand empty;
 BTSpecialCommand *thenCmd = &empty;
 BTSpecialCommand *elseCmd = &empty;
 if (thenClause.ops.size() == 1)
 {
  thenCmd = dynamic_cast<BTSpecialCommand*>(thenClause.ops[0]);
  if (NULL == thenCmd)
   throw FileException("Then clause not a simple command.");
 }
 if (elseClause.ops.size() == 1)
 {
  elseCmd = dynamic_cast<BTSpecialCommand*>(elseClause.ops[0]);
  if (NULL == elseCmd)
   throw FileException("Else clause not a simple command.");
 }
 f.writeShort(type);
 strcpy(tmp, text);
 memset(tmp + len, 0, 26 - len);
 f.writeUByteArray(26, (IUByte *)tmp);
 f.writeShort(number);
 thenCmd->write(f);
 elseCmd->write(f);
}

BTSpecial::BTSpecial()
{
 name = new char[1];
 name[0] = 0;
}

BTSpecial::BTSpecial(const BTSpecial &copy)
 : flags(copy.flags), body(copy.body)
{
 name = new char[strlen(copy.name) + 1];
 strcpy(name, copy.name);
}

BTSpecial::BTSpecial(BinaryReadFile &f)
{
 char tmp[26];
 IUByte unknown;
 BitField labelNeeded;

 f.readUByteArray(25, (IUByte *)tmp);
 tmp[25] = 0;
 name = new char[strlen(tmp) + 1];
 strcpy(name, tmp);
 f.readUByte(unknown);
 int nothing = 0;
 for (int i = 0; i < 20; i++)
 {
  IShort type;
  char tmp[27];
  IShort number;
  f.readShort(type);
  f.readUByteArray(26, (IUByte *)tmp);
  tmp[26] = 0;
  f.readShort(number);
  BTSpecialCommand *opThen = new BTSpecialCommand;
  opThen->read(f);
  BTSpecialCommand *opElse = new BTSpecialCommand;
  opElse->read(f);
  switch (type)
  {
   case -99:
    delete opThen;
    delete opElse;
    break;
   case -1:
    if (opThen->getType() == BTSPECIALCOMMAND_NOTHING)
    {
     ++nothing;
    }
    else
    {
     while (nothing > 0)
     {
      body.addOperation(new BTSpecialCommand(BTSPECIALCOMMAND_NOTHING));
      --nothing;
     }
     body.addOperation(opThen);
     if (opThen->getType() == BTSPECIALCOMMAND_GOTO)
     {
      labelNeeded.set(opThen->getNumber(0));
      snprintf(tmp, 26, "line number %d", opThen->getNumber(0));
      opThen->setText(tmp);
     }
    }
    delete opElse;
    break;
   default:
   {
    while (nothing > 0)
    {
     body.addOperation(new BTSpecialCommand(BTSPECIALCOMMAND_NOTHING));
     --nothing;
    }
    BTSpecialConditional *op = new BTSpecialConditional(type, tmp, number);
    if (opThen->getType() == BTSPECIALCOMMAND_GOTO)
    {
     labelNeeded.set(opThen->getNumber(0));
     snprintf(tmp, 26, "line number %d", opThen->getNumber(0));
     opThen->setText(tmp);
    }
    op->addThenOperation(opThen);
    if (opElse->getType() == BTSPECIALCOMMAND_GOTO)
    {
     labelNeeded.set(opElse->getNumber(0));
     snprintf(tmp, 26, "line number %d", opElse->getNumber(0));
     opElse->setText(tmp);
    }
    op->addElseOperation(opElse);
    body.addOperation(op);
    break;
   }
  }
  if (-99 == type)
  {
   break;
  }
 }
 for (int i = labelNeeded.getMaxSet(); i > 0; --i)
 {
  if (labelNeeded.isSet(i))
  {
   BTSpecialCommand *op = new BTSpecialCommand(BTSPECIALCOMMAND_LABEL);
   snprintf(tmp, 26, "line number %d", i);
   op->setText(tmp);
   if (body.ops.size() <= i - 1)
    body.addOperation(op);
   else
    body.insertOperation(i - 1, op);
  }
 }
}

BTSpecial::~BTSpecial()
{
 if (name)
  delete [] name;
}

BTSpecialBody *BTSpecial::getBody()
{
 return &body;
}

const char *BTSpecial::getName() const
{
 return name;
}

std::string BTSpecial::printFlags() const
{
 std::string results;
 for (int i = 0; i < BT_SPECIALFLAGS; ++i)
 {
  if (flags.isSet(i))
  {
   if (results.length() > 0)
    results += " ";
   results += specialFlag[i];
  }
 }
 return results;
}

void BTSpecial::print(FILE *f) const
{
 int i, last;

 fprintf(f, "%s\n", name);
 body.print(f, 0);
}

void BTSpecial::run(BTDisplay &d) const
{
 BTGame::getGame()->addFlags(d, flags);
 try
 {
  bool stop = false;
  int line = 0;
  while (!stop)
  {
   try
   {
    body.runFromLine(d, line);
    stop = true;
   }
   catch (const BTSpecialGoto &g)
   {
    line = body.findLabel(g.label);
   }
  }
 }
 catch (const BTSpecialStop &)
 {
 }
}

void BTSpecial::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("flag", &flags, &specialFlagLookup);
 s->add("body", &body);
}

void BTSpecial::setName(const std::string &nm)
{
 delete name;
 name = new char[nm.length() + 1];
 strcpy(name, nm.c_str());
}

void BTSpecial::write(BinaryWriteFile &f)
{
 char tmp[26];
 IUByte unknown;
 BitField labelCmd;

 if (strlen(name) > 25)
  throw FileException(std::string("Special name '") + name + std::string("' is too large."));
 int i;
 memset(tmp, 0, 26);
 strcpy(tmp, name);
 f.writeUByteArray(26, (IUByte *)tmp);
 for (i = 0; i < body.ops.size(); ++i)
 {
  BTSpecialCommand *cmd = dynamic_cast<BTSpecialCommand*>(body.ops[i]);
  if ((cmd) && (cmd->getType() == BTSPECIALCOMMAND_LABEL))
   labelCmd.set(i);
 }
 if (body.ops.size() - labelCmd.count() > 20)
  throw FileException(std::string("Too many commands in the special '") + name + std::string("'."));
 for (i = 0; i < body.ops.size(); ++i)
 {
  BTSpecialConditional *conditional = dynamic_cast<BTSpecialConditional*>(body.ops[i]);
  if (conditional)
  {
   BTSpecialCommand *cmd = dynamic_cast<BTSpecialCommand*>(conditional->getThenClause()->getOperation(0));
   if ((cmd) && (cmd->getType() == BTSPECIALCOMMAND_GOTO))
   {
    int line = body.findLabel(cmd->getText());
    cmd->setNumber(0, line + 1 - (line ? labelCmd.count(0, line - 1) : 0));
   }
   cmd = dynamic_cast<BTSpecialCommand*>(conditional->getElseClause()->getOperation(0));
   if ((cmd) && (cmd->getType() == BTSPECIALCOMMAND_GOTO))
   {
    int line = body.findLabel(cmd->getText());
    cmd->setNumber(0, line + 1 - (line ? labelCmd.count(0, line - 1) : 0));
   }
  }
  else
  {
   BTSpecialCommand *cmd = dynamic_cast<BTSpecialCommand*>(body.ops[i]);
   if ((cmd) && (cmd->getType() == BTSPECIALCOMMAND_GOTO))
   {
    int line = body.findLabel(cmd->getText());
    cmd->setNumber(0, line + 1 - (line ? labelCmd.count(0, line - 1) : 0));
   }
  }
 }
 for (i = 0; i < body.ops.size(); ++i)
 {
  if (labelCmd.isSet(i))
   continue;
  BTSpecialConditional *conditional = dynamic_cast<BTSpecialConditional*>(body.ops[i]);
  if (conditional)
  {
   conditional->write(f);
  }
  else
  {
   BTSpecialCommand *cmd = dynamic_cast<BTSpecialCommand*>(body.ops[i]);
   if (cmd)
   {
    IShort type(-1);
    f.writeShort(type);
    memset(tmp, 0, 26);
    f.writeUByteArray(26, (IUByte *)tmp);
    type = 0;
    f.writeShort(type);
    cmd->write(f);
    cmd->write(f);
   }
   else
    throw FileException("Unhandled special operation");
  }
 }
 if (body.ops.size() - labelCmd.count() < 20)
 {
  IShort type(-99);
  f.writeShort(type);
  memset(tmp, 0, 26);
  f.writeUByteArray(26, (IUByte *)tmp);
  type = 0;
  f.writeShort(type);
  BTSpecialCommand emptyCmd;
  emptyCmd.write(f);
  emptyCmd.write(f);
 }
}

void BTSpecial::upgrade()
{
 BitField labelNeeded;
 body.upgradeToLabel(labelNeeded);
 char tmp[27];
 for (int i = labelNeeded.getMaxSet(); i > 0; --i)
 {
  if (labelNeeded.isSet(i))
  {
   BTSpecialCommand *op = new BTSpecialCommand(BTSPECIALCOMMAND_LABEL);
   snprintf(tmp, 26, "line number %d", i);
   op->setText(tmp);
   if (body.ops.size() <= i - 1)
    body.addOperation(op);
   else
    body.insertOperation(i - 1, op);
  }
 }
}

BTMonsterChance::BTMonsterChance(int c /*= 0*/, int g /*= 1*/)
 : chance(c), groups(g)
{
}

std::string BTMonsterChance::createString()
{
 char tmp[40];
 snprintf(tmp, 40, "%d%% of %d group%s", getChance(), getGroups(), ((getGroups() > 1) ? "s" : ""));
 return tmp;
}

int BTMonsterChance::getChance() const
{
 return chance;
}

int BTMonsterChance::getGroups() const
{
 return groups;
}

void BTMonsterChance::serialize(ObjectSerializer* s)
{
 s->add("chance", &chance);
 s->add("groups", &groups);
}

BTMap::BTMap(BinaryReadFile &f)
 : filename(0)
{
 IUByte unknown;
 IShort t;
 char tmp[26];

 f.readUByteArray(25, (IUByte *)tmp);
 tmp[25] = 0;
 name = new char[strlen(tmp) + 1];
 strcpy(name, tmp);
 f.readUByte(unknown);
 f.readShort(t);
 type = t;
 f.readShort(level);
 f.readShort(monsterLevel);
 IShort chance;
 f.readShort(chance);
 monsterChance.push_back(new BTMonsterChance(chance, 1));
 if ((BTMAPTYPE_CITY == type) || (BTMAPTYPE_WILDERNESS == type))
  light = 5;
 else
  light = 0;
 // Ignore filename
 f.readUByteArray(9, (IUByte *)tmp);
 f.readUByte(unknown);
 xSize = 22;
 ySize = 22;
 for (int y = 0; y < 22; y++)
 {
  for (int x = 0; x < 22; x++)
  {
   BTMapSquare *sq = new BTMapSquare;
   square.push_back(sq);
   sq->read(f);
  }
 }
 int i;
 try {
  for (i = 0; i < 30; i++)
  {
   specials.push_back(new BTSpecial(f));
  }
 }
 catch (FileException e)
 {
 }
}

BTMap::BTMap(int v /*= 2*/)
 : name(NULL), version(v), light(0), filename(NULL)
{
}

BTMap::~BTMap()
{
 if (name)
  delete [] name;
 if (filename)
  delete [] filename;
}

void BTMap::addSpecial(BTSpecial *s)
{
 specials.push_back(s);
}

void BTMap::setSpecial(IShort x, IShort y, IShort special)
{
 square[y * xSize + x]->setSpecial(special);
}

void BTMap::checkRandomEncounter(BTDisplay &d) const
{
 int chance = 0;
 int roll = BTDice(1, 100).roll();
 for (int i = 0; i < monsterChance.size(); ++i)
 {
  chance += monsterChance[i]->getChance();
  if (roll <= chance)
  {
   generateRandomEncounter(d, monsterChance[i]->getGroups());
   break;
  }
 }
}

const char *BTMap::getFilename() const
{
 return filename;
}

IShort BTMap::getLevel() const
{
 return level;
}

int BTMap::getLight() const
{
 return light;
}

int BTMap::getMonsterChance() const
{
 int chance = 0;
 for (int i = 0; i < monsterChance.size(); ++i)
 {
  chance += monsterChance[i]->getChance();
 }
 return chance;
}

IShort BTMap::getMonsterLevel() const
{
 return monsterLevel;
}

const char *BTMap::getName() const
{
 return name;
}

int BTMap::getNumOfSpecials() const
{
 return specials.size();
}

void BTMap::generateRandomEncounter(BTDisplay &d, int groups) const
{
 BTGame *game = BTGame::getGame();
 BTFactory<BTMonster> &monList = game->getMonsterList();
 std::vector<int> monsters;
 for (int i = 0; i < monList.size(); ++i)
 {
  if ((monList[i].getLevel() == getMonsterLevel()) && (monList[i].isWandering()))
  {
   monsters.push_back(i);
  }
 }
 if (monsters.size() > 0)
 {
  for (int i = groups; i > 0; --i)
  {
   int monIndex = BTDice(1, monsters.size(), -1).roll();
   game->getCombat().addEncounter(monsters[monIndex]);
  }
  game->getCombat().run(d);
 }
}

BTSpecial *BTMap::getSpecial(IShort num)
{
 return ((specials.size() > num) ? specials[num] : NULL);
}

BTMapSquare &BTMap::getSquare(IShort y, IShort x)
{
 while (y < 0)
  y += ySize;
 y = y % ySize;
 while (x < 0)
  x += xSize;
 x = x % xSize;
 return *square[y * xSize + x];
}

IShort BTMap::getType() const
{
 return type;
}

IShort BTMap::getXSize() const
{
 return xSize;
}

IShort BTMap::getYSize() const
{
 return ySize;
}

void BTMap::resize(IShort newXSize, IShort newYSize)
{
 if (newYSize < ySize)
 {
  square.erase(square.begin() + (newYSize * xSize), square.end());
  ySize = newYSize;
  for (int i = 0; i < xSize; ++i)
  {
   getSquare(ySize - 1, i).setWall(BTDIRECTION_SOUTH, getSquare(0, i).getWall(BTDIRECTION_NORTH));
  }
 }
 if (newXSize < xSize)
 {
  for (int i = 0; i < ySize; ++i)
  {
   square.erase(square.begin() + (i * newXSize + newXSize), square.begin() + (i * newXSize + xSize));
  }
  xSize = newXSize;
  for (int i = 0; i < ySize; ++i)
  {
   getSquare(i, xSize - 1).setWall(BTDIRECTION_EAST, getSquare(i, 0).getWall(BTDIRECTION_WEST));
  }
 }
 else if (newXSize > xSize)
 {
  for (int i = 0; i < ySize; ++i)
  {
   for (int k = xSize; k < newXSize; ++k)
   {
    square.insert(square.begin() + (i * newXSize + k), new BTMapSquare);
   }
  }
  int oldXSize = xSize;
  xSize = newXSize;
  for (int i = 0; i < ySize; ++i)
  {
   getSquare(i, oldXSize).setWall(BTDIRECTION_WEST, getSquare(i, oldXSize - 1).getWall(BTDIRECTION_EAST));
   getSquare(i, xSize - 1).setWall(BTDIRECTION_EAST, getSquare(i, 0).getWall(BTDIRECTION_WEST));
  }
 }
 if (newYSize > ySize)
 {
  for (int i = ySize; i < newYSize; ++i)
  {
   for (int k = 0; k < xSize; ++k)
    square.push_back(new BTMapSquare);
  }
  int oldYSize = ySize;
  ySize = newYSize;
  for (int i = 0; i < xSize; ++i)
  {
   getSquare(oldYSize, i).setWall(BTDIRECTION_NORTH, getSquare(oldYSize - 1, i).getWall(BTDIRECTION_SOUTH));
   getSquare(ySize - 1, i).setWall(BTDIRECTION_SOUTH, getSquare(0, i).getWall(BTDIRECTION_NORTH));
  }
 }
}

void BTMap::setFilename(const char *f)
{
 filename = new char[strlen(f) + 1];
 strcpy(filename, f);
}

void BTMap::serialize(ObjectSerializer* s)
{
 s->add("name", &name);
 s->add("version", &version);
 s->add("type", &type, NULL, &BTCore::getCore()->getPsuedo3DConfigList());
 s->add("level", &level);
 s->add("xSize", &xSize);
 s->add("ySize", &ySize);
 s->add("monsterChance", &monsterChance, &BTMonsterChance::create);
 s->add("monsterLevel", &monsterLevel);
 s->add("light", &light);
 s->add("square", &square, &BTMapSquare::create);
 s->add("special", &specials, &BTSpecial::create);
}

void BTMap::write(BinaryWriteFile &f)
{
 IUByte unknown;
 IShort t;
 char tmp[26];

 if (strlen(name) > 25)
  throw FileException(std::string("Map name '") + name + std::string("' is too large."));
 if (xSize > 22)
  throw FileException("X size is too large.");
 if (ySize > 22)
  throw FileException("Y size is too large.");
 int len = strlen(filename);
 if ((len <= 4) || (strcmp(".MAP", filename + (len - 4)) != 0))
  throw FileException("Filename does not end in \".MAP\".");
 if (len - 4 > 8)
  throw FileException("Filename is too long.");
 if (specials.size() > 30)
  throw FileException("Too many specials.");
 memset(tmp, 0, 26);
 strcpy(tmp, name);
 f.writeUByteArray(26, (IUByte *)tmp);
 t = type;
 f.writeShort(t);
 f.writeShort(level);
 f.writeShort(monsterLevel);
 IShort chance = getMonsterChance();
 f.writeShort(chance);
 // Ignore filename
 strcpy(tmp, filename);
 memset(tmp + len - 4, 0, 10 - len + 4);
 for (int i = len - 4; i < 10; ++i)
  tmp[i] = 0;
 f.writeUByteArray(10, (IUByte *)tmp);
 BTMapSquare empty;
 for (int y = 0; y < 22; y++)
 {
  for (int x = 0; x < 22; x++)
  {
   if ((x >= getXSize()) && (y >= getYSize()))
    empty.write(f);
   else
    getSquare(y, x).write(f);
  }
 }
 int i;
 for (i = 0; i < specials.size(); i++)
 {
  specials[i]->write(f);
 }
}

void BTMap::upgrade()
{
 if (version == 1)
 {
  for (int i = 0; i < specials.size(); i++)
  {
   specials[i]->upgrade();
  }
 }
 version = 2;
}

