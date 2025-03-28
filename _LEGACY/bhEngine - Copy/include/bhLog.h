#ifndef BH_LOG_H
#define BH_LOG_H

#define BH_LOG_MESSAGE_SIZ 1024 << 1

#ifdef __cplusplus
extern "C" {
#endif

  typedef enum //Matching SDL_LogPriority, see: SDL_log.h
  {
    LP_VERBOSE = 1,
    LP_DEBUG,
    LP_INFO,
    LP_WARN,
    LP_ERROR,
    LP_CRITICAL,

    LOG_NUM_PRIORITIES
  }
  bhLogPriority;

  void bhLog_Message(bhLogPriority type, const char* format, ...);

#ifdef __cplusplus
}
#endif //extern "C"

#endif //BH_LOG_H
