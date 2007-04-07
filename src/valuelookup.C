/*-------------------------------------------------------------------------*\
  <valuelookup.C> -- Lookup implementation file

  Date      Programmer  Description
  04/07/07  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "valuelookup.h"

std::string ArrayLookup::getName(int index)
{
 if ((index > 0) && (index < size))
  return array[index];
 else
  return "";
}

int ArrayLookup::getIndex(std::string name)
{
 for (int i = 0; i < size; ++i)
 {
  if (0 == strcmp(name.c_str(), array[i]))
   return i;
 }
 return -1;
}
