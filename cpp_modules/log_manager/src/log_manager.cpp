#include "log_manager.h"

#include <iostream>

LogManager::LogManager(
    const std::string &filePath)
    : filePath_(filePath),
      logFile_(filePath_, std::ios::app),
      consoleEnabled_(true),
      autoFlush_(true),
      maxCachedLogs_(10),
      recentLogs_{},
      logCounts_{
          {LogLevel::Info, 0},
          {LogLevel::Warning, 0},
          {LogLevel::Error, 0}}
{
    if (logFile_.is_open())
    {
        std::cout
            << "[INFO] Log file opened: "
            << filePath_
            << '\n';
    }
    else
    {
        std::cerr
            << "[ERROR] Failed to open log file: "
            << filePath_
            << '\n';
    }
}

LogManager::~LogManager()
{
    /*
     * 正常设计中，所有工作线程都应在 logger
     * 析构前完成 join。
     */
    std::lock_guard<std::mutex> lock(mutex_);

    if (logFile_.is_open())
    {
        if (autoFlush_)
        {
            logFile_.flush();
        }

        logFile_.close();
    }

    if (consoleEnabled_)
    {
        std::cout
            << "[INFO] Log file closed: "
            << filePath_
            << '\n';
    }
}

LogManager &LogManager::setConsoleEnabled(
    bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);

    consoleEnabled_ = enabled;

    return *this;
}

LogManager &LogManager::setAutoFlush(
    bool enabled)
{
    std::lock_guard<std::mutex> lock(mutex_);

    autoFlush_ = enabled;

    return *this;
}

LogManager &LogManager::setMaxCachedLogs(
    std::size_t maxCount)
{
    std::lock_guard<std::mutex> lock(mutex_);

    maxCachedLogs_ = maxCount;

    if (maxCachedLogs_ == 0)
    {
        recentLogs_.clear();

        return *this;
    }

    while (recentLogs_.size() >
           maxCachedLogs_)
    {
        recentLogs_.erase(
            recentLogs_.begin());
    }

    return *this;
}

void LogManager::log(
    LogLevel level,
    const std::string &message)
{
    const std::string line =
        "[" +
        levelToString(level) +
        "] " +
        message;

    /*
     * 一次 log() 内所有共享资源操作
     * 放在同一个临界区中。
     */
    std::lock_guard<std::mutex> lock(mutex_);

    if (consoleEnabled_)
    {
        std::cout
            << line
            << '\n';
    }

    if (logFile_.is_open())
    {
        logFile_
            << line
            << '\n';

        if (autoFlush_)
        {
            logFile_.flush();
        }
    }

    cacheLogLine(line);

    ++logCounts_[level];
}

bool LogManager::isFileOpen() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return logFile_.is_open();
}

const std::string &LogManager::getFilePath() const
{
    /*
     * filePath_ 在构造完成后不会再改变，
     * 因此可以直接返回 const 引用。
     */
    return filePath_;
}

std::vector<std::string>
LogManager::getRecentLogs() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return recentLogs_;
}

void LogManager::clearRecentLogs()
{
    std::lock_guard<std::mutex> lock(mutex_);

    recentLogs_.clear();
}

std::size_t LogManager::getLogCount(
    LogLevel level) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    const auto iterator =
        logCounts_.find(level);

    if (iterator == logCounts_.end())
    {
        return 0;
    }

    return iterator->second;
}

std::map<LogLevel, std::size_t>
LogManager::getAllLogCounts() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return logCounts_;
}

void LogManager::resetLogCounts()
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto &entry : logCounts_)
    {
        entry.second = 0;
    }
}

std::string LogManager::levelToString(
    LogLevel level) const
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

void LogManager::cacheLogLine(
    const std::string &line)
{
    if (maxCachedLogs_ == 0)
    {
        return;
    }

    recentLogs_.push_back(line);

    while (recentLogs_.size() >
           maxCachedLogs_)
    {
        recentLogs_.erase(
            recentLogs_.begin());
    }
}