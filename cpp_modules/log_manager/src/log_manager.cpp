#include "log_manager.h"

#include <iostream>

LogManager::LogManager(const std::string &filePath)
    :filePath_(filePath),
    logFile_(filePath,std::ios::app),
    consoleEnabled_(true),
    autoFlush_(true),
    maxCachedLogs_(5),
    recentLogs_()
{
    if(!this->logFile_.is_open())
    {
        std::cerr<<"[ERROR] Failed to open log file:"
        <<filePath_
        <<'\n';
        return;
    }
    std::cout<<"[INFO] Log file opened:"
        <<this->filePath_
        <<'\n';
}
LogManager::~LogManager()
{
    if(this->logFile_.is_open())
    {
        this->logFile_.flush();
        this->logFile_.close();

        std::cout<<"[INFO] Log file closed:"
            <<this->filePath_
            <<'\n';
    }
}
LogManager &LogManager::setConsoleEnabled(bool enabled)
{
    this->consoleEnabled_=enabled;
    return *this;
}
LogManager &LogManager::setAutoFlush(bool enabled)
{
    this->autoFlush_=enabled;

    return *this;
}


LogManager &LogManager::setMaxCachedLogs(std::size_t maxCount)
{
    this->maxCachedLogs_=maxCount;

    if(this->maxCachedLogs_==0)
    {
        this->recentLogs_.clear();
        return *this;
    }
    while (this->recentLogs_.size()>this->maxCachedLogs_)
    {
        this->recentLogs_.erase(this->recentLogs_.begin());
        /* code */
    }
    return *this;
    
}
std::string LogManager::levelToString(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

void LogManager::cacheLogLine(const std::string &line)
{
    if(this->maxCachedLogs_==0)
    {
        return;
    }
    this->recentLogs_.push_back(line);

    while (this->recentLogs_.size()>this->maxCachedLogs_)
    {
        this->recentLogs_.erase(this->recentLogs_.begin());
    }
}

void LogManager::log(LogLevel level, const std::string &message)
{
    const std::string line=
    "["+this->levelToString(level)+"] "+message;

    this->cacheLogLine(line);
    
    if(this->consoleEnabled_)
    {
        std::cout<<line<<'\n';
    }
    if(this->logFile_.is_open())
    {
        this->logFile_<<line<<'\n';
    
        if(this->autoFlush_)
        {
        this->logFile_.flush();
        }
    }
}
bool LogManager::isFileOpen() const
{
    return logFile_.is_open();
}

const std::string &LogManager::getFiledPath()const
{
    return this->filePath_;
}
const std::vector<std::string> &LogManager::getRecentLogs() const
{
    return this->recentLogs_;
}

void LogManager::clearRecentLogs()
{
    this->recentLogs_.clear();
}