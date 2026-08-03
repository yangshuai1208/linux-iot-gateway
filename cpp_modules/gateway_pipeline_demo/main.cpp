#include "log_manager.h"
#include "protocol_parser.h"

#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace
{
    std::queue<std::string>commandQueue;

    std::mutex queueMutex;
    
    std::condition_variable queueCondition;

    bool inputFinished=false;

    struct PipelineStats
    {
        int accepted{0};
        int rejected{0};
    };
    void inputProducer(LogManager &logger)
    {
        const std::vector<std::string>testInputs
        {
            "OPEN",
            "grab\r\n",
            "JUMP",
            "\tSTOP\t",
            "release\n"
        };
        int sequence=0;

        for(const std::string &rawInput:testInputs)
        {
            ++sequence;
            {
                std::lock_guard<std::mutex>lock(queueMutex);

                commandQueue.push(rawInput);
            }
            logger.log(LogLevel::Info,"Input producer queued command #"+std::to_string(sequence));

            queueCondition.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        {
            std::lock_guard<std::mutex>lock(queueMutex);
            inputFinished=true;
        }
        logger.log(LogLevel::Info,"Input producer finished ");

        queueCondition.notify_all();
    }
    void commandConsumer(ProtocolParser &parser,LogManager &logger,PipelineStats&stats)
    {
        while(true)
        {
            std::unique_lock<std::mutex>lock(queueMutex);

            queueCondition.wait(lock,
                                    []
                                {
                                    return !commandQueue.empty()||
                                    inputFinished;
                                });
            if(commandQueue.empty()&&inputFinished)
            {
                break;
            }
            const std::string rawInput=commandQueue.front();

            commandQueue.pop();

            lock.unlock();

            const Command command=parser.parse(rawInput);

            if(!parser.isValid(command))
            {
                ++stats.rejected;

                logger.log(LogLevel::Warning,"Rejected invalid gateway command");
            
                continue;
            }
            const std::string commandName=parser.commandToString(command);

            const std::string executorCommand=parser.toExecutorCommand(command);

            ++stats.accepted;
      logger.log(
    LogLevel::Info,
    "Gateway command: " +
        commandName +
        " -> " +
        executorCommand);
        }
        logger.log(LogLevel::Info,"Command consumer finished");
    }
}
int main()
{
    LogManager logger(
        "gateway_pipeline_demo.log");

    ProtocolParser parser;

    PipelineStats stats;

    logger.setConsoleEnabled(true)
          .setAutoFlush(true)
          .setMaxCachedLogs(50);

    if (!logger.isFileOpen())
    {
        std::cerr
            << "[ERROR] Logger initialization failed"
            << '\n';

        return 1;
    }

    std::cout
        << "[MAIN] start gateway pipeline demo"
        << '\n';

    std::thread producer(
        inputProducer,
        std::ref(logger));

    std::thread consumer(
        commandConsumer,
        std::ref(parser),
        std::ref(logger),
        std::ref(stats));

    if (producer.joinable())
    {
        producer.join();
    }

    if (consumer.joinable())
    {
        consumer.join();
    }

    std::cout
        << "[RESULT] accepted="
        << stats.accepted
        << '\n';

    std::cout
        << "[RESULT] rejected="
        << stats.rejected
        << '\n';

    std::cout
        << "[RESULT] INFO logs="
        << logger.getLogCount(
               LogLevel::Info)
        << '\n';

    std::cout
        << "[RESULT] WARNING logs="
        << logger.getLogCount(
               LogLevel::Warning)
        << '\n';

    std::cout
        << "[MAIN] all worker threads joined"
        << '\n';

    return 0;
}