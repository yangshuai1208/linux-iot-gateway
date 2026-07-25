#include "log_manager.h"

#include <iostream>

LogManager::LogManager(const std::string &filePath)
    :filePath_(filePath),
    logFile_(filePath,std::ios::app),
    consoleEnabled_(true),
    autoFlush_(true)
{
    if(!logFile_.is_open())
    {
        std::cerr<<"[ERROR] Failed to open log file:"
        <<filePath_
        <<'\n';
        return;
    }
    std::cout<<"[INFO] Log file opened:"
        <<filePath_
        <<'\n';
}
LogManager::~LogManager()
{
    if(logFile_.is_open())
    {
        logFile_.flush();
        logFile_.close();

        std::cout<<"[INFO] Log file closed:"
            <<filePath_
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
void LogManager::log(LogLevel level, const std::string &message)
{
    const std::string line=
    "["+this->levelToString(level)+"] "+message;

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