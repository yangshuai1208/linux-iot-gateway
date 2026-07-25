#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <fstream>
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
    explicit LogManager(const std::string &filePath);

    ~LogManager();
    
    LogManager &setConsoleEnabled(bool enabled);

    LogManager &setAutoFlush(bool enabled);

    void log(LogLevel level, const std::string &message);

    bool isFileOpen() const;

    const std::string &getFiledPath() const;
private:
    std::string levelToString(LogLevel level) const;

    std::string filePath_;
    std::ofstream logFile_;

    bool consoleEnabled_;
    bool autoFlush_;

};

#endif
