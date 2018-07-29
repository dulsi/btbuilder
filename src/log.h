#ifndef __LOG_H
#define __LOG_H
/*-------------------------------------------------------------------------*\
  <log.h> -- Log header file

  Date      Programmer  Description
  07/28/18  Dennis      Created.
\*-------------------------------------------------------------------------*/

#include <string>

#define LOG(l, msg) \
 if ((Log::logger) && (Log::logger->isLogging(l))) \
 { \
  Log::logger->log(l, msg); \
 }

class Log
{
 public:
  enum level { trace, debug, error, info };

  Log(level l, const std::string &name);
  
  ~Log();

  bool isLogging(level l);

  void log(level l, const std::string &msg);

  level lvl;
  FILE *logFile;
  static Log *logger;
};

#endif
