#include <Log.h>
#include <gtest/gtest.h>

TEST(LoggerTest, PrintLog) {
    LOGI("Logging with LogLevel: %s", "INFO");
    LOGV("Logging with LogLevel: %s", "VERBOSE");
    LOGD("Logging with LogLevel: %s", "DEBUG");
    LOGW("Logging with LogLevel: %s", "WARN");
    LOGE("Logging with LogLevel: %s", "ERROR");
    LOGF("Logging with LogLevel: %s", "FATAL");
}
