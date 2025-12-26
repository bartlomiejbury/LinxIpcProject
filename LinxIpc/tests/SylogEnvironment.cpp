#include <gtest/gtest.h>
#include <syslog.h>
#include "trace.h"

class SyslogEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        TRACE_INIT();
        syslog(LOG_INFO, "=== tests started ===");
    }

    void TearDown() override {
        syslog(LOG_INFO, "=== tests finished ===");
        TRACE_CLOSE();
    }
};

::testing::Environment* const syslog_env =
    ::testing::AddGlobalTestEnvironment(new SyslogEnvironment);