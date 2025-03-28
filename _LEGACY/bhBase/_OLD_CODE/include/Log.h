#ifndef BH_LOG_H
#define BH_LOG_H

#include "Types.h"

////////////////////////////////////////////////////////////////////////////////
namespace bh{

void ReportSystem(char const* msg,char const* details = 0);
void ReportError(char const* msg,char const* details = 0);
void ReportGLError(char const* msg,char const* details = 0);
void ReportWarning(char const* msg,char const* details = 0);
void DumpLogToConsole();
void DumpLogToFile(char const* fileName);

};
////////////////////////////////////////////////////////////////////////////////
#endif
