#pragma once

typedef enum { SEVERITY_DEBUG, SEVERITY_INFO, SEVERITY_WARNING, SEVERITY_ERROR, SEVERITY_END } LogSeverity;


#if defined USE_LOGGING && USE_LOGGING >= SEVERITY_DEBUG && USE_LOGGING < SEVERITY_END

    #ifdef __cplusplus
    extern "C" {
    #endif
    void trace(LogSeverity severity, const char *fileName, int lineNum, const char *format, ...);
    #ifdef __cplusplus
    }
    #endif

    #define LOG(SEVERITY, ...)                                                                                             \
        if (SEVERITY >= USE_LOGGING && SEVERITY < SEVERITY_END) {                                                          \
            trace(SEVERITY, __FILE__, __LINE__, ##__VA_ARGS__);                                                            \
        }

#else

    #define LOG(SEVERITY, ...)

#endif

#define LOG_INFO(...) LOG(SEVERITY_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(SEVERITY_DEBUG, __VA_ARGS__)
#define LOG_WARNING(...) LOG(SEVERITY_WARNING, __VA_ARGS__)
#define LOG_ERROR(...) LOG(SEVERITY_ERROR, __VA_ARGS__)
#define LOG_ENTER() LOG(SEVERITY_DEBUG, "%s enter", __func__)
#define LOG_EXIT() LOG(SEVERITY_DEBUG, "%s exit", __func__)
