#ifndef WORKSPACE_INCLUDE_LOG_H
#define WORKSPACE_INCLUDE_LOG_H

#include <Logger.h>
#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define LOG(fmt, ...) LOGI(fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) Logger::printLog(LogLevel::INFO, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGV(fmt, ...) Logger::printLog(LogLevel::VERBOSE, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) Logger::printLog(LogLevel::DEBUG, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) Logger::printLog(LogLevel::WARN, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) Logger::printLog(LogLevel::ERROR, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define LOGF(fmt, ...) Logger::printLog(LogLevel::FATAL, __FILENAME__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif  // WORKSPACE_INCLUDE_LOG_H