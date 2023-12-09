#include "trace.h"
#include <cstdarg>
#include <syslog.h>
#include <string.h>
#include <unordered_map>

#ifdef USE_LINX_LOGGING
static int dummyLogger = (openlog(NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1), 0);
#endif

static const std::unordered_map<LogSeverity, int> severityMap{
    {LogSeverity::SEVERITY_DEBUG, LOG_DEBUG},     {LogSeverity::SEVERITY_INFO, LOG_INFO}, 
    {LogSeverity::SEVERITY_WARNING, LOG_WARNING}, {LogSeverity::SEVERITY_ERROR, LOG_ERR}};


void trace(LogSeverity severity, const char *fileName, int lineNum, const char *format, ...) {
    std::va_list argptr;

    va_start(argptr, format);

    char formatBuffer[1000];
    const char *file = strrchr(fileName, '/') ? strrchr(fileName, '/') + 1 : fileName;

    sprintf(formatBuffer, "(%s:%d): %s", file, lineNum, format);
    vsyslog(severityMap.at(severity), formatBuffer, argptr);

    va_end(argptr);
}
