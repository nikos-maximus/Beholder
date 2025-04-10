#ifndef BH_LOG_HPP
#define BH_LOG_HPP

#define BH_LOG_MESSAGE_SIZ 1024 << 1

typedef enum bhLogPriority //Matching SDL_LogPriority, see: SDL_log.h
{
  //LP_INVALID,
  //LP_TRACE,
  LP_VERBOSE = 2,
  LP_DEBUG,
  LP_INFO,
  LP_WARN,
  LP_ERROR,
  LP_CRITICAL,

  LOG_NUM_PRIORITIES
};

void bhLog_Message(int category, bhLogPriority type, const char* format, ...);

#endif //BH_LOG_HPP
