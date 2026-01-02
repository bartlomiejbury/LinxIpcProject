#pragma once

// Default logging level if not specified by build system
// Can be overridden with -DTRACE_LEVEL=4 at compile time
#ifndef TRACE_LEVEL
#define TRACE_LEVEL 0
#endif

#define SEVERITY_ERROR 1
#define SEVERITY_WARNING 2
#define SEVERITY_INFO 3
#define SEVERITY_DEBUG 4

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

#if TRACE_LEVEL >= SEVERITY_ERROR
    extern "C" void trace_init();
    extern "C" void trace_close();
    #define TRACE_INIT() trace_init()
    #define TRACE_CLOSE() trace_close()
    #define TRACE_ERROR(...) trace_error(__FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define TRACE_INIT()
    #define TRACE_CLOSE()
    #define TRACE_ERROR(...)
#endif

#if TRACE_LEVEL >= SEVERITY_WARNING
    #define TRACE_WARNING(...) trace_warning(__FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define TRACE_WARNING(...)
#endif

#if TRACE_LEVEL >= SEVERITY_INFO
    #define TRACE_INFO(...) trace_info(__FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define TRACE_INFO(...)
#endif

#if TRACE_LEVEL >= SEVERITY_DEBUG
    #define TRACE_DEBUG(...) trace_debug(__FILE__, __LINE__, ##__VA_ARGS__)
    #define TRACE_ENTER() trace_debug(__FILE__, __LINE__, "%s enter", __func__)
    #define TRACE_EXIT() trace_debug(__FILE__, __LINE__, "%s exit", __func__)
#else
    #define TRACE_DEBUG(...)
    #define TRACE_ENTER()
    #define TRACE_EXIT()
#endif

#if TRACE_LEVEL > SEVERITY_DEBUG
    #error "Cannot set log level > SEVERITY_DEBUG"
#endif
