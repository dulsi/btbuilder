/*-------------------------------------------------------------------------*\
  <valuelookup.C> -- Lookup implementation file

  Date      Programmer  Description
  04/07/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "valuelookup.h"
#include <string.h>

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

