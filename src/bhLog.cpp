#include <SDL3/SDL_log.h>
#include <stdio.h>
#include "bhLog.hpp"

namespace bhLog
{
  void Message(Category c, Priority p, const char* format, ...)
  {
    static char g_logString[BH_LOG_MESSAGE_SIZ];

    // TODO:
    // a. See how to enrich output using SDL_LOG_CATEGORY_
    // b. Can output to, say xml file as planned using SDL_LogSetOutputFunction

    va_list args;
    va_start(args, format);
    vsprintf_s(g_logString, BH_LOG_MESSAGE_SIZ, format, args);
    va_end(args);
    SDL_LogMessage(static_cast<SDL_LogCategory>(c), static_cast<SDL_LogPriority>(p), g_logString);
  }
}
