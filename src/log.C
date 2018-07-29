/*-------------------------------------------------------------------------*\
  <log.C> -- Log implementation file

  Date      Programmer  Description
  07/28/18  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include "log.h"

Log *Log::logger(0);

Log::Log(Log::level l, const std::string &name)
 : lvl(l), logFile(0)
{
 logFile = fopen(name.c_str(), "a+");
 logger = this;
}

Log::~Log()
{
 logger = 0;
}

bool Log::isLogging(Log::level l)
{
 return (l >= lvl);
}

void Log::log(level l, const std::string &msg)
{
 fprintf(logFile, "%s\n", msg.c_str());
}
