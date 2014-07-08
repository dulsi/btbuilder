/*-------------------------------------------------------------------------*\
  <dice.C> -- Dice implementation file

  Date      Programmer  Description
  11/07/00  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "dice.h"

#include <ctime>
#include <sstream>

#define DICE_NUMBERMASK 0x1F
#define DICE_TYPEMASK   0xE0

IShort BTDice::validType[DICE_VALIDTYPES] = {2, 4, 6, 8, 10, 12, 20, 100};
base_generator_type BTDice::generator(42u);

BTDice::BTDice(BinaryReadFile &f)
{
 read(f);
}

BTDice::BTDice(int n, int t, int m /*= 0*/)
 : number(n), type(t), modifier(m)
{
}

BTDice::BTDice()
 : number(1), type(validType[0]), modifier(0)
{
}

std::string BTDice::createString()
{
 std::ostringstream stream;
 if (number != 0)
  stream << number << "d" << type;
 if (modifier < 0)
  stream << modifier;
 else if (modifier > 0)
  stream << "+" << modifier;
 return stream.str();
}

int BTDice::getMax() const
{
 return modifier + type * number;
}

int BTDice::getMin() const
{
 return modifier + number;
}

int BTDice::getModifier() const
{
 return modifier;
}

int BTDice::getNumber() const
{
 return number;
}

int BTDice::getType() const
{
 return type;
}

void BTDice::read(BinaryReadFile &f)
{
 IUByte b;

 f.readUByte(b);
 number = (b & DICE_NUMBERMASK) + 1;
 type = validType[(b & DICE_TYPEMASK) >> 5];
}

int BTDice::roll() const
{
 int r = modifier;
 if (type > 0)
 {
  boost::uniform_int<> ran_dist(1, type);
  boost::variate_generator<base_generator_type&, boost::uniform_int<> > ran(generator, ran_dist);
  for (int i = 0; i < number; ++i)
   r += ran();
 }
 return r;
}

void BTDice::serialize(ObjectSerializer* s)
{
 s->add("n", &number);
 s->add("d", &type);
 s->add("m", &modifier);
}

void BTDice::setModifier(IShort val)
{
 modifier = val;
}

void BTDice::setNumber(IShort val)
{
 number = val;
}

void BTDice::setType(IShort val)
{
 type = val;
}

void BTDice::write(BinaryWriteFile &f)
{
 IUByte b;

 for (b = 0; b < DICE_VALIDTYPES; b++)
 {
  if (type == validType[b])
  {
   break;
  }
 }
 b = (b << 5) + (number - 1);
 f.writeUByte(b);
}

void BTDice::Init()
{
 generator.seed(static_cast<unsigned int>(std::time(0)));
}

