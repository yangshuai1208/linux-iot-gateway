#include "log_manager.h"

#include <iostream>

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
void LogManager::log(LogLevel level, const std::string &message)const
{
    std::cout<< "["<< levelToString(level)<< "]"
        <<message
        <<'\n';
}