#include "bhLog.h"
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>

////////////////////////////////////////////////////////////////////////////////
static const char* g_logTags[LOG_NUM_PRIORITIES] =
{
    "LOG:",
    "DEBUG: ",
    "INFO: ",
    "WARNING: ",
    "ERROR: ",
    "CRITICAL: "
};

static char g_logString[BH_LOG_MESSAGE_SIZ];

void bhLog_WriteToConsole(bhLogPriority type, const char* string, char endl)
{
    switch (type)
    {
        case LP_VERBOSE:
        case LP_INFO:
        {
            printf("\x1b[m%s%c", string, endl);
            break;
        }
        case LP_WARN:
        case LP_DEBUG:
        {
            printf("\x1b[33m%s%c", string, endl);
            break;
        }
        case LP_ERROR:
        case LP_CRITICAL:
        {
            printf("\x1b[31m%s%c", string, endl);
            break;
        }
        case LOG_NUM_PRIORITIES:
        default:
        {
            break;
        }
    }
}

void bhLog_Message(bhLogPriority type, const char* format, ...)
{
    sprintf_s(g_logString, BH_LOG_MESSAGE_SIZ, "%s ", g_logTags[type]);
    bhLog_WriteToConsole(type, g_logString, 0);

    va_list args;
    va_start(args, format);
    vsprintf_s(g_logString, BH_LOG_MESSAGE_SIZ, format, args);
    va_end(args);
    bhLog_WriteToConsole(type, g_logString, '\n');
}
