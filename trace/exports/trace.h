#pragma once

typedef enum { SEVERITY_ERROR=1, SEVERITY_WARNING, SEVERITY_INFO, SEVERITY_DEBUG  } LogSeverity;

#define TRACE_INIT()
#define TRACE_RAW(SEVERITY, file, line, format, va_args)
#define TRACE_INFO(...)
#define TRACE_DEBUG(...)
#define TRACE_WARNING(...)
#define TRACE_ERROR(...)
#define TRACE_ENTER()
#define TRACE_EXIT()

#if USE_LOGGING >= 1
    #ifdef __cplusplus
    extern "C" {
    #endif

    #include <stdarg.h>

    void traceInit();
    void vtrace(LogSeverity severity, const char *fileName, int lineNum, const char *format, va_list argptr);
    void trace(LogSeverity severity, const char *fileName, int lineNum, const char *format, ...);

    #ifdef __cplusplus
    }
    #endif

    #undef TRACE_INIT
    #define TRACE_INIT() traceInit()

    #undef TRACE_RAW
    #define TRACE_RAW(SEVERITY, file, line, format, va_args) vtrace(SEVERITY, file, line, format, va_args)

    #undef TRACE_ERROR
    #define TRACE_ERROR(...) trace(SEVERITY_ERROR, __FILE__, __LINE__, ##__VA_ARGS__)

    #if USE_LOGGING >= 2
        #undef TRACE_WARNING
        #define TRACE_WARNING(...) trace(SEVERITY_WARNING, __FILE__, __LINE__, ##__VA_ARGS__)
    #endif

    #if USE_LOGGING >= 3
        #undef TRACE_INFO
        #define TRACE_INFO(...) trace(SEVERITY_INFO, __FILE__, __LINE__, ##__VA_ARGS__)
    #endif

    #if USE_LOGGING >= 4
        #undef TRACE_DEBUG
        #define TRACE_DEBUG(...) trace(SEVERITY_DEBUG, __FILE__, __LINE__, ##__VA_ARGS__)

        #undef TRACE_ENTER
        #define TRACE_ENTER() trace(SEVERITY_DEBUG, "%s enter", __func__)

        #undef TRACE_EXIT
        #define TRACE_EXIT() trace(SEVERITY_DEBUG, "%s exit", __func__)
    #endif

    #if USE_LOGGING >= 5
        #error "Cannot set log level > 4"
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

void trace_debug(const char *fileName, int lineNum, const char *format, ...);
void trace_info(const char *fileName, int lineNum, const char *format, ...);
void trace_warning(const char *fileName, int lineNum, const char *format, ...);
void trace_error(const char *fileName, int lineNum, const char *format, ...);

#ifdef __cplusplus
}
#endif
