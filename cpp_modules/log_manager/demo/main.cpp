#include "log_manager.h"

#include <iostream>
#include <map>
#include <string>

int main()
{
    LogManager logger("log_manager_demo.log");

    logger.setConsoleEnabled(true)
          .setAutoFlush(true)
          .setMaxCachedLogs(3);

    if (!logger.isFileOpen())
    {
        std::cerr << "[ERROR] Logger initialization failed"
                  << '\n';

        return 1;
    }

    logger.log(LogLevel::Info,
               "Linux IoT Gateway started");

    logger.log(LogLevel::Info,
               "Sensor data received");

    logger.log(LogLevel::Warning,
               "MQTT Broker is not connected");

    logger.log(LogLevel::Error,
               "Failed to parse sensor data");

    logger.log(LogLevel::Info,
               "Gateway is retrying");

    /*
     * 查询单个日志等级的统计数量。
     */
    std::cout << "[INFO] INFO count: "
              << logger.getLogCount(LogLevel::Info)
              << '\n';

    std::cout << "[INFO] WARNING count: "
              << logger.getLogCount(LogLevel::Warning)
              << '\n';

    std::cout << "[INFO] ERROR count: "
              << logger.getLogCount(LogLevel::Error)
              << '\n';

    /*
     * 使用 const 引用获取全部统计信息，
     * 避免复制整个 map。
     */
    const std::map<LogLevel, std::size_t> &logCounts =
        logger.getAllLogCounts();

    std::cout << "[INFO] All log statistics:"
              << '\n';

    /*
     * 遍历 map：
     * entry.first  是日志等级；
     * entry.second 是统计数量。
     */
    for (const auto &entry : logCounts)
    {
        std::string levelName;

        switch (entry.first)
        {
        case LogLevel::Info:
            levelName = "INFO";
            break;

        case LogLevel::Warning:
            levelName = "WARNING";
            break;

        case LogLevel::Error:
            levelName = "ERROR";
            break;

        default:
            levelName = "UNKNOWN";
            break;
        }

        std::cout << "  "
                  << levelName
                  << ": "
                  << entry.second
                  << '\n';
    }

    /*
     * 清零日志等级统计。
     */
    logger.resetLogCounts();

    std::cout << "[INFO] INFO count after reset: "
              << logger.getLogCount(LogLevel::Info)
              << '\n';

    std::cout << "[INFO] WARNING count after reset: "
              << logger.getLogCount(LogLevel::Warning)
              << '\n';

    std::cout << "[INFO] ERROR count after reset: "
              << logger.getLogCount(LogLevel::Error)
              << '\n';

    return 0;
}