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
    
    void log(LogLevel level, const std::string &message);

    bool isFileOpen() const;
private:
    std::string levelToString(LogLevel level) const;

    std::string filePath_;
    std::ofstream logFile_;

};

#endif
