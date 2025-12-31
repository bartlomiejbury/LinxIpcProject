#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <syslog.h>
#include <unordered_map>
#include "trace.h"
#include <sys/syscall.h>
#include <unistd.h>

//LCOV_EXCL_START

static const std::unordered_map<int, int> severityMap{{SEVERITY_DEBUG, LOG_DEBUG},
                                                      {SEVERITY_INFO, LOG_INFO},
                                                      {SEVERITY_WARNING, LOG_WARNING},
                                                      {SEVERITY_ERROR, LOG_ERR}};

static void vtrace(int severity, const char *fileName, int lineNum, const char *format, va_list argptr) {

    pid_t tid = syscall(SYS_gettid);

    char formatBuffer[1000];
    const char *file = strrchr(fileName, '/');
    file = file ? file + 1 : fileName;

    snprintf(formatBuffer, sizeof(formatBuffer), "[tid=%d](%s:%d): %s", tid, file, lineNum, format);
    vsyslog(severityMap.at(severity), formatBuffer, argptr);
}

void trace_init() {
    int defaultSeverity = USE_LOGGING;
    openlog( NULL, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER );

    if (const char* env_severity = std::getenv( "LOG_LEVEL" )) {
        int num = atoi( env_severity );

        if (severityMap.find(num) != severityMap.end()) {
            defaultSeverity = num;
        }
    }

    setlogmask( LOG_UPTO( severityMap.at( defaultSeverity ) ) );
    TRACE_INFO( "Set LOG_LEVEL: %d", defaultSeverity );
}

void trace_close() {
    closelog();
}

void trace_error(const char *fileName, int lineNum, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_ERROR, fileName, lineNum, format, argptr);
    va_end(argptr);
}

void trace_warning(const char *fileName, int lineNum, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_WARNING, fileName, lineNum, format, argptr);
    va_end(argptr);
}

void trace_info(const char *fileName, int lineNum, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_INFO, fileName, lineNum, format, argptr);
    va_end(argptr);
}

void trace_debug(const char *fileName, int lineNum, const char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vtrace(SEVERITY_DEBUG, fileName, lineNum, format, argptr);
    va_end(argptr);
}

//LCOV_EXCL_STOP
