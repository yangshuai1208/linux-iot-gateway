#include "log_manager.h"

#include <iostream>
#include <string>
#include <vector>

int main()
{
    LogManager logger("log_manager_demo.log");

    logger.setConsoleEnabled(true)
        .setAutoFlush(true)
        .setMaxCachedLogs(3);

    
    if(!logger.isFileOpen())
    {
        std::cerr<<"[ERROR] Logger initialization failed"
                 <<'\n';
        return 1;
    }

    logger.log(LogLevel::Info,
                "Linux Iot Gateway Start");
    logger.log(LogLevel::Info,
                "Sensor data received");

    logger.log(LogLevel::Warning,
                "MQTT Broker is not connected");


    logger.log(LogLevel::Error,
                "Failed to parse sensor data");
    
    logger.log(LogLevel::Info,
                "Gateway is retrying");

    const std::vector<std::string>&recentLogs=
    logger.getRecentLogs();

    std::cout<<"[INFO] Cache log count:"
    <<recentLogs.size()
    <<'\n';

    if(recentLogs.empty())
    {
        std::cout<<"[INFO] Recent log cache is empty"
        <<'\n';
    }
    else
    {
        std::cout<<"[INFO] Recent logs:"
        <<'\n';

    for(const std::string &line:recentLogs)
    {
        std::cout<<""<<line<<'\n';
    }
    }
    logger.clearRecentLogs();

    std::cout<<"[INFO] Cached log count after clear"
    <<logger.getRecentLogs().size()
    <<'\n';

    return 0;
    
}