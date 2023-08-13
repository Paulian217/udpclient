#include <Logger.h>

#include <cstdarg>
#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>

enum Color : uint8_t { NONE = 0x00, BLACK = 0x01, RED = 0x02, GREEN = 0x03, YELLOW = 0x04, BLUE = 0x05, MAGENTA = 0x06, CYAN = 0x07, WHITE = 0x08 };
enum Face : uint8_t { NORMAL = 0x00, BOLD = 0x01, DARK = 0x02, ULINE = 0x04, INVERT = 0x07, INVISIBLE = 0x08, CLINE = 0x09 };

const char* GetTimeStamp() {
    static char buff[64];
    static time_t tt;
    static struct tm* t;

    tt = time(NULL);
    t = localtime(&tt);

    strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", t);
    return buff;
}

static std::stringstream printColorLog(const LogLevel& level) {
    std::stringstream strstream;
    switch (level) {
        case LogLevel::NONE:
            strstream << "\e[" << Color::NONE << "m";
            break;
        case LogLevel::VERBOSE:
            strstream << "\e[" << Color::BLACK << "m";
            break;
        case LogLevel::INFO:
            strstream << "\e[" << Color::GREEN + 29 << "m";
            break;
        case LogLevel::DEBUG:
            strstream << "\e[" << Color::BLUE + 29 << "m";
            break;
        case LogLevel::WARN:
            strstream << "\e[" << Color::YELLOW + 29 << "m";
            break;
        case LogLevel::ERROR:
            strstream << "\e[" << Color::RED + 29 << "m";
            break;
        case LogLevel::FATAL:
            strstream << "\e[" << Color::RED + 29 << "m"
                      << "\e[01m";
            break;
        default:
            break;
    }
    return strstream;
}

void Logger::printLog(const LogLevel& level, const char* file, const char* func, const int line, const char* fmt, ...) {
    static std::mutex mutex;
    va_list args;
    va_start(args, fmt);
    auto nLength = vsnprintf(nullptr, 0, fmt, args) + 1;
    auto buffer = new char[nLength];
    vsnprintf(buffer, nLength, fmt, args);
    va_end(args);
    {
        std::lock_guard<std::mutex> lock(mutex);
        std::cout << printColorLog(level).str() << GetTimeStamp() << " [" << file << ":" << line << "] " << buffer << "\e[0m" << std::endl;
    }
    delete[] buffer;
}