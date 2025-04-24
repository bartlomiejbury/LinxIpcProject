#pragma once

#ifdef LINX_LOG_PREFIX
    #define DECL_STR(PREFIX, SUFFIX) DECL_STR2(PREFIX, SUFFIX)
    #define DECL_STR2(PREFIX, SUFIX) extern "C" void PREFIX##_##SUFIX(const char *fileName, int lineNum, const char *format, ...);

    #define CALL_STR(PREFIX, SUFFIX, ...) CALL_STR2(PREFIX, SUFFIX, ##__VA_ARGS__)
    #define CALL_STR2(PREFIX, SUFIX, ...) PREFIX##_##SUFIX(__FILE__, __LINE__, ##__VA_ARGS__)

    DECL_STR(LINX_LOG_PREFIX, error)
    DECL_STR(LINX_LOG_PREFIX, warning)
    DECL_STR(LINX_LOG_PREFIX, info)
    DECL_STR(LINX_LOG_PREFIX, debug)

    #define LINX_ERROR(...) CALL_STR(LINX_LOG_PREFIX, error, ##__VA_ARGS__)
    #define LINX_WARNING(...) CALL_STR(LINX_LOG_PREFIX, warning, ##__VA_ARGS__)
    #define LINX_INFO(...) CALL_STR(LINX_LOG_PREFIX, info, ##__VA_ARGS__)
    #define LINX_DEBUG(...) CALL_STR(LINX_LOG_PREFIX, debug, ##__VA_ARGS__)
#else
    #define LINX_INFO(...)
    #define LINX_DEBUG(...)
    #define LINX_WARNING(...)
    #define LINX_ERROR(...)
#endif
