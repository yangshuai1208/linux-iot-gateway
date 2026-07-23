#include "log_manager.h"

int main()
{
    LogManager logger;

    logger.log(LogLevel::Info,
                "Linux Iot Gateway Start");

    logger.log(LogLevel::Warning,
                "MQTT Broker is not connected");


    logger.log(LogLevel::Error,
                "Failed to parse sensor data");
    
    return 0;
    
}