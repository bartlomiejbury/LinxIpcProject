#include "trace.h"
#include <syslog.h>
#include <string.h>
#include <unordered_map>
#include <stdio.h>
#include <cstdlib>
#include <stdarg.h>

//LCOV_EXCL_START
#if USE_LOGGING >= SEVERITY_ERROR

static const std::unordered_map<int, int> severityMap{{SEVERITY_DEBUG, LOG_DEBUG},
                                                      {SEVERITY_INFO, LOG_INFO},
                                                      {SEVERITY_WARNING, LOG_WARNING},
                                                      {SEVERITY_ERROR, LOG_ERR}};

static void vtrace(int severity, const char *fileName, int lineNum, const char *format, va_list argptr) {
    char formatBuffer[1000];
    const char *file = strrchr(fileName, '/');
    file = file ? file + 1 : fileName;

    sprintf(formatBuffer, "(%s:%d): %s", file, lineNum, format);
    vsyslog(severityMap.at(severity), formatBuffer, argptr);
}

void trace_init() {
    int defaultSeverity = USE_LOGGING;
    openlog( NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1 );

    if (const char* env_severity = std::getenv( "LOG_LEVEL" )) {
        int num = atoi( env_severity );

        if (severityMap.find(num) != severityMap.end()) {
            defaultSeverity = num;
        }
    }

    setlogmask( LOG_UPTO( severityMap.at( defaultSeverity ) ) );
    TRACE_INFO( "Set LOG_LEVEL: %d", defaultSeverity );
}
#endif

void trace_error(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= SEVERITY_ERROR
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_ERROR, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_warning(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= SEVERITY_WARNING
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_WARNING, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_info(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= SEVERITY_INFO
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_INFO, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

void trace_debug(const char *fileName, int lineNum, const char *format, ...) {
#if USE_LOGGING >= SEVERITY_DEBUG
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_DEBUG, fileName, lineNum, format, argptr);
    va_end(argptr);
#endif
}

//LCOV_EXCL_STOP
