#include "log_manager.h"

#include <iostream>

int main()
{
    LogManager logger("log_manager_demo.log");

    logger.setConsoleEnabled(true)
        .setAutoFlush(true);

    std::cout<<"[INFO] Current log file:"
             <<logger.getFiledPath()
             <<'\n';

    
    if(!logger.isFileOpen())
    {
        std::cerr<<"[ERROR] Logger initialization failed"
                 <<'\n';
        return 1;
    }
    logger.log(LogLevel::Info,
                "Linux Iot Gateway Start");

    logger.log(LogLevel::Warning,
                "MQTT Broker is not connected");


    logger.log(LogLevel::Error,
                "Failed to parse sensor data");
        
    logger.setConsoleEnabled(false);

    logger.log(LogLevel::Info,
    "This message is written only to file");

    logger.setConsoleEnabled(true);

    logger.log(LogLevel::Info,
            "Console output restored");

    return 0;
    
}