#include "trace.h"
#include <syslog.h>
#include <string.h>
#include <unordered_map>
#include <stdio.h>
#include <cstdlib>
#include <cstring>

#if USE_LOGGING >= 1

static const std::unordered_map<LogSeverity, int> severityMap{{LogSeverity::SEVERITY_DEBUG, LOG_DEBUG},
                                                              {LogSeverity::SEVERITY_INFO, LOG_INFO},
                                                              {LogSeverity::SEVERITY_WARNING, LOG_WARNING},
                                                              {LogSeverity::SEVERITY_ERROR, LOG_ERR}};

//LCOV_EXCL_START
void traceInit() {

    LogSeverity defaultSeverity = static_cast<LogSeverity>(USE_LOGGING);
    openlog( NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1 );

    if (const char* env_severity = std::getenv( "LOG_LEVEL" )) {
        int num = atoi( env_severity );

        if (num >= LogSeverity::SEVERITY_DEBUG && num <= LogSeverity::SEVERITY_ERROR) {
            defaultSeverity = static_cast<LogSeverity>(num);
        }
    }

    setlogmask( LOG_UPTO( severityMap.at( defaultSeverity ) ) );
    TRACE_INFO( "Set LOG_LEVEL: %d", defaultSeverity );
}

void vtrace(LogSeverity severity, const char *fileName, int lineNum, const char *format, va_list argptr) {

    char formatBuffer[1000];
    const char *file = strrchr(fileName, '/');
    file = file ? file + 1 : fileName;

    sprintf(formatBuffer, "(%s:%d): %s", file, lineNum, format);
    vsyslog(severityMap.at(severity), formatBuffer, argptr);
}

void trace(LogSeverity severity, const char *fileName, int lineNum, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vtrace(severity, fileName, lineNum, format, argptr);
    va_end(argptr);
}
//LCOV_EXCL_STOP
#endif

void trace_error(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= 1
    va_list argptr;
    va_start(argptr, format);
    TRACE_RAW(SEVERITY_ERROR, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_warning(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= 2
    va_list argptr;
    va_start(argptr, format);
    TRACE_RAW(SEVERITY_WARNING, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_info(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= 3
    va_list argptr;
    va_start(argptr, format);
    TRACE_RAW(SEVERITY_INFO, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_debug(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= 4
    va_list argptr;
    va_start(argptr, format);
    TRACE_RAW(SEVERITY_DEBUG, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}
