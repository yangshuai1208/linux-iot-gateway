#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

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

    LogManager &setMaxCachedLogs(std::size_t maxCount);

    void log(LogLevel level, const std::string &message);

    bool isFileOpen() const;

    const std::string &getFiledPath() const;

    const std::vector<std::string> &getRecentLogs()const;

    void clearRecentLogs();
private:
    std::string levelToString(LogLevel level) const;

    void cacheLogLine(const std::string &line);

    std::string filePath_;
    std::ofstream logFile_;

    bool consoleEnabled_;
    bool autoFlush_;

    std::size_t maxCachedLogs_;
    std::vector<std::string> recentLogs_;

};

#endif
