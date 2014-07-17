/*-------------------------------------------------------------------------*\
  <valuelookup.C> -- Lookup implementation file

  Date      Programmer  Description
  04/07/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "valuelookup.h"
#include <string.h>

int ValueLookup::getEndIndex()
{
 return size();
}

int ValueLookup::getFirstIndex()
{
 return 0;
}

int ValueLookup::getNextIndex(int previous)
{
 return previous + 1;
}

std::string ArrayLookup::getName(int index)
{
 if ((index >= 0) && (index < sz))
  return array[index];
 else
  return "";
}

int ArrayLookup::getIndex(std::string name)
{
 for (int i = 0; i < sz; ++i)
 {
  if (0 == strcmp(name.c_str(), array[i]))
   return i;
 }
 return -1;
}

size_t ArrayLookup::size()
{
 return sz;
}

std::string PairLookup::getName(int index)
{
 int arrayIndex = getArrayFromIndex(index);
 if ((arrayIndex >= 0) && (arrayIndex < sz))
  return names[arrayIndex];
 else
  return "";
}

int PairLookup::getIndex(std::string name)
{
 for (int i = 0; i < sz; ++i)
 {
  if (0 == strcmp(name.c_str(), names[i]))
   return value[i];
 }
 return -1;
}

size_t PairLookup::size()
{
 return sz;
}

int PairLookup::getEndIndex()
{
 return value[size() - 1] + 1;
}

int PairLookup::getFirstIndex()
{
 return value[0];
}

int PairLookup::getNextIndex(int previous)
{
 int arrayIndex = getArrayFromIndex(previous);
 if ((arrayIndex >= 0) && (arrayIndex + 1 < sz))
  return value[arrayIndex + 1];
 else
  return getEndIndex();
}

int PairLookup::getArrayFromIndex(int index)
{
 for (int i = 0; i < sz; ++i)
 {
  if (value[i] == index)
   return i;
 }
 return -1;
}

