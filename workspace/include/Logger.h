#ifndef WORKSPACE_INCLUDE_LOGGER_H
#define WORKSPACE_INCLUDE_LOGGER_H

enum class LogLevel { NONE, INFO, VERBOSE, DEBUG, WARN, ERROR, FATAL };

class Logger {
public:
    static void printLog(const LogLevel& level, const char* file, const char* func, const int line, const char* fmt, ...);
};

#endif  // WORKSPACE_INCLUDE_LOGGER_H