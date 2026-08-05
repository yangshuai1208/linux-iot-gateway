#include "log_manager.h"
#include "protocol_parser.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace
{
    constexpr int kMaxRetries=1;

    constexpr auto kAckTimeout=std::chrono::milliseconds(300);

    struct PendingRequest
    {
        std::uint32_t sequence;
        std::string payload;
        int attempt;
    };

    struct  AckFrame
    {
        std::uint32_t sequence;
        bool success;
        std::string message;
    };
    struct RetryStats
    {
        int sentFrames{0};
        int acked{0};
        int timeouts{0};
        int retries{0};
        int failed{0};
    };

    std::queue<PendingRequest>txQueue;
    std::mutex txMutex;
    std::condition_variable txCondition;

    bool transmissionFinished=false;

    std::map<std::uint32_t,AckFrame>ackMap;
    std::mutex ackMutex;
    std::condition_variable ackCondition;

    void simulatedDevice(LogManager &logger)
    {
        while(true)
        {
            std::unique_lock<std::mutex>lock(txMutex);

            txCondition.wait(lock,
                                []
                                {
                                    return !txQueue.empty()||transmissionFinished;
                                }); 
            if(txQueue.empty()&&transmissionFinished)
            {
                break;
            }
            const PendingRequest request=txQueue.front();

            txQueue.pop();
            lock.unlock();

            logger.log(LogLevel::Info,"Device received seq="+std::to_string(request.sequence)+"attempt="+std::to_string(request.attempt));

            if(request.sequence==3&&request.attempt==1)
            {
                logger.log(LogLevel::Warning,"Simulated ACK loss for seq=3");

                continue;
            }
    std::this_thread::sleep_for(
            std::chrono::milliseconds(120));

        AckFrame ack{
            request.sequence,
            true,
            "OK"};

        {
            std::lock_guard<std::mutex> ackLock(
                ackMutex);

            ackMap[ack.sequence] = ack;
        }

        ackCondition.notify_all();
    }

    logger.log(
        LogLevel::Info,
        "Simulated device finished");
}

void commandSender(
    ProtocolParser &parser,
    LogManager &logger,
    RetryStats &stats)
{
    const std::vector<std::string> commands{
        "OPEN",
        "GRAB",
        "STOP",
        "RELEASE"};

    std::uint32_t sequence = 0;

    for (const std::string &payload : commands)
    {
        ++sequence;

        const Command command =
            parser.parse(payload);

        if (!parser.isValid(command))
        {
            continue;
        }

        bool completed = false;

        for (int attempt = 1;
             attempt <= kMaxRetries + 1;
             ++attempt)
        {
            PendingRequest request{
                sequence,
                payload,
                attempt};

            {
                std::lock_guard<std::mutex> lock(
                    txMutex);

                txQueue.push(request);
            }

            ++stats.sentFrames;

            logger.log(
                LogLevel::Info,
                "Send seq=" +
                    std::to_string(sequence) +
                    " attempt=" +
                    std::to_string(attempt));

            txCondition.notify_one();

            /*
             * 等待指定 sequence 的 ACK。
             */
            std::unique_lock<std::mutex> ackLock(
                ackMutex);

            const bool received =
                ackCondition.wait_for(
                    ackLock,
                    kAckTimeout,
                    [sequence]
                    {
                        return ackMap.find(sequence) !=
                               ackMap.end();
                    });

            if (received)
            {
                const AckFrame ack =
                    ackMap.at(sequence);

                ackMap.erase(sequence);

                ackLock.unlock();

                ++stats.acked;

                logger.log(
                    LogLevel::Info,
                    "ACK received seq=" +
                        std::to_string(
                            ack.sequence));

                std::cout
                    << "[ACK] seq="
                    << ack.sequence
                    << " message="
                    << ack.message
                    << '\n';

                completed = true;
                break;
            }

            ackLock.unlock();

            ++stats.timeouts;

            std::cout
                << "[TIMEOUT] seq="
                << sequence
                << " attempt="
                << attempt
                << '\n';

            if (attempt <= kMaxRetries)
            {
                ++stats.retries;

                std::cout
                    << "[RETRY] seq="
                    << sequence
                    << '\n';

                continue;
            }

            ++stats.failed;

            logger.log(
                LogLevel::Error,
                "Request failed seq=" +
                    std::to_string(sequence));
        }

        if (!completed)
        {
            std::cout
                << "[FAILED] seq="
                << sequence
                << '\n';
        }
    }

    {
        std::lock_guard<std::mutex> lock(
            txMutex);

        transmissionFinished = true;
    }

    txCondition.notify_all();

    logger.log(
        LogLevel::Info,
        "Command sender finished");
}

} // namespace

int main()
{
    LogManager logger(
        "gateway_timeout_demo.log");

    ProtocolParser parser;

    RetryStats stats;

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
        << "[MAIN] start timeout retry demo"
        << '\n';

    std::thread device(
        simulatedDevice,
        std::ref(logger));

    std::thread sender(
        commandSender,
        std::ref(parser),
        std::ref(logger),
        std::ref(stats));

    if (sender.joinable())
    {
        sender.join();
    }

    if (device.joinable())
    {
        device.join();
    }

    std::cout
        << "[RESULT] sent_frames="
        << stats.sentFrames
        << '\n';

    std::cout
        << "[RESULT] acked="
        << stats.acked
        << '\n';

    std::cout
        << "[RESULT] timeouts="
        << stats.timeouts
        << '\n';

    std::cout
        << "[RESULT] retries="
        << stats.retries
        << '\n';

    std::cout
        << "[RESULT] failed="
        << stats.failed
        << '\n';

    return 0;
}