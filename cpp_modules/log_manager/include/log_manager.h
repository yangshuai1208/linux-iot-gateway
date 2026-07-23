#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <string>

enum class LogLevel
{
    Info = 0,
    Warning,
    Error
};

class LogManager
{
public:
    void log(LogLevel level, const std::string &message) const;

private:
    std::string levelToString(LogLevel level) const;
};

#endif
