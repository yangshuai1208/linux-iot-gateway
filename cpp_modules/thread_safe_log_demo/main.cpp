#include "log_manager.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace{
    constexpr int kLogsPerWorker=5;

    void writeLogs(
        LogManager &logger,
        LogLevel level,
        const std::string &workerName
    )
{
    for(int index=1; index<=kLogsPerWorker;++index)
    {
        std::ostringstream message;

        message
        <<"worker="
        <<workerName
        <<"thread"
        <<std::this_thread::get_id()
        <<"sequence="
        <<index;

        logger.log(level,message.str());

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
}
int main()
{
    LogManager logger("thread_safe_log_demo.log");

    logger.setConsoleEnabled(true)
          .setAutoFlush(true)
          .setMaxCachedLogs(20);
    if(!logger.isFileOpen())
    {
        std::cerr
        <<"[ERROR] Logger initialization failed"
        <<'\n';

        return 1;
    }

    std::thread collector(
    writeLogs,
    std::ref(logger),
    LogLevel::Info,
    "collector");

std::thread processor(
    writeLogs,
    std::ref(logger),
    LogLevel::Warning,
    "processor");

    if(collector.joinable())
    {
        collector.join();
    }
    if(processor.joinable())
    {
        processor.join();
    }
    const std::size_t infoCount=logger.getLogCount(LogLevel::Info);

    const std::size_t warningCount=logger.getLogCount(LogLevel::Warning);

     const std::size_t errorCount=logger.getLogCount(LogLevel::Error);

    const std::vector<std::string> recentLogs=logger.getRecentLogs();

    std::cout
    <<"[RESULT] INFO="
    <<infoCount
    <<'\n';

   std::cout
    <<"[RESULT] WARNING="
    <<warningCount
    <<'\n';
    
   std::cout
    <<"[RESULT] ERROR="
    <<errorCount
    <<'\n';
    
   std::cout
    <<"[RESULT] cahced_logs="
    <<recentLogs.size()
    <<'\n';

   std::cout
    <<"[MAIN]all worker threads joined"
    <<'\n';

    return 0;
        


}   