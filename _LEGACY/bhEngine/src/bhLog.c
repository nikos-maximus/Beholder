#include "bhLog.h"
#include <SDL3/SDL_log.h>
#include <stdio.h>

void bhLog_Message(bhLogPriority type, const char* format, ...)
{
  static char g_logString[BH_LOG_MESSAGE_SIZ];

  // TODO:
  // a. See how to enrich output using SDL_LOG_CATEGORY_
  // b. Can output to, say xml file as planned using SDL_LogSetOutputFunction

  va_list args;
  va_start(args, format);
  vsprintf_s(g_logString, BH_LOG_MESSAGE_SIZ, format, args);
  va_end(args);
  SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, type, g_logString);
}
