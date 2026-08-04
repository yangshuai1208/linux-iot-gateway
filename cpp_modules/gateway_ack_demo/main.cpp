#include "log_manager.h"
#include "protocol_parser.h"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace
{

struct RawCommand
{
    std::uint32_t sequence;
    std::string source;
    std::string payload;
};

enum class ResultCode
{
    Ok = 0,
    InvalidCommand,
    ExecutionFailed
};

struct CommandResult
{
    std::uint32_t sequence;
    ResultCode code;
    std::string command;
    std::string executorCommand;
    std::string message;
};

struct PipelineStats
{
    int received{0};
    int accepted{0};
    int rejected{0};
    int ackCount{0};
};

std::queue<RawCommand> commandQueue;

std::mutex queueMutex;

std::condition_variable queueCondition;

bool inputFinished = false;

int resultCodeToInt(ResultCode code)
{
    switch (code)
    {
    case ResultCode::Ok:
        return 0;

    case ResultCode::InvalidCommand:
        return 1001;

    case ResultCode::ExecutionFailed:
        return 1002;

    default:
        return -1;
    }
}

std::string resultCodeToString(ResultCode code)
{
    switch (code)
    {
    case ResultCode::Ok:
        return "OK";

    case ResultCode::InvalidCommand:
        return "INVALID_COMMAND";

    case ResultCode::ExecutionFailed:
        return "EXECUTION_FAILED";

    default:
        return "UNKNOWN_RESULT";
    }
}

void printResult(
    const CommandResult &result)
{
    if (result.code == ResultCode::Ok)
    {
        std::cout
            << "[ACK]"
            << " seq="
            << result.sequence
            << " code="
            << resultCodeToInt(result.code)
            << " result="
            << resultCodeToString(result.code)
            << " command="
            << result.command
            << " executor="
            << result.executorCommand
            << '\n';

        return;
    }

    std::cout
        << "[ERROR]"
        << " seq="
        << result.sequence
        << " code="
        << resultCodeToInt(result.code)
        << " result="
        << resultCodeToString(result.code)
        << " message="
        << result.message
        << '\n';
}

void inputProducer(
    LogManager &logger,
    PipelineStats &stats)
{
    const std::vector<std::string> testInputs{
        "OPEN",
        " grab\r\n",
        "JUMP",
        "\tSTOP\t",
        "release\n"};

    std::uint32_t sequence = 0;

    for (const std::string &payload : testInputs)
    {
        ++sequence;

        RawCommand rawCommand{
            sequence,
            "SIMULATED_UART",
            payload};

        {
            std::lock_guard<std::mutex> lock(
                queueMutex);

            commandQueue.push(rawCommand);

            ++stats.received;
        }

        logger.log(
            LogLevel::Info,
            "Queued command seq=" +
                std::to_string(sequence));

        queueCondition.notify_one();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));
    }

    {
        std::lock_guard<std::mutex> lock(
            queueMutex);

        inputFinished = true;
    }

    logger.log(
        LogLevel::Info,
        "Input producer finished");

    queueCondition.notify_all();
}

void commandConsumer(
    ProtocolParser &parser,
    LogManager &logger,
    PipelineStats &stats)
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(
            queueMutex);

        queueCondition.wait(
            lock,
            []
            {
                return !commandQueue.empty() ||
                       inputFinished;
            });

        if (commandQueue.empty() &&
            inputFinished)
        {
            break;
        }

        const RawCommand rawCommand =
            commandQueue.front();

        commandQueue.pop();

        lock.unlock();

        const Command command =
            parser.parse(rawCommand.payload);

        if (!parser.isValid(command))
        {
            ++stats.rejected;
            ++stats.ackCount;

            CommandResult result{
                rawCommand.sequence,
                ResultCode::InvalidCommand,
                "UNKNOWN",
                "HAND_NONE",
                "invalid gateway command"};

            logger.log(
                LogLevel::Warning,
                "Rejected command seq=" +
                    std::to_string(
                        rawCommand.sequence));

            printResult(result);

            continue;
        }

        const std::string commandName =
            parser.commandToString(command);

        const std::string executorCommand =
            parser.toExecutorCommand(command);

        /*
         * 当前仍为模拟执行：
         * 这里只表示命令已经完成网关侧解析和映射，
         * 不是 STM32 的真实 ACK。
         */
        CommandResult result{
            rawCommand.sequence,
            ResultCode::Ok,
            commandName,
            executorCommand,
            "command accepted"};

        ++stats.accepted;
        ++stats.ackCount;

        logger.log(
            LogLevel::Info,
            "Command seq=" +
                std::to_string(
                    rawCommand.sequence) +
                " " +
                commandName +
                " -> " +
                executorCommand);

        printResult(result);
    }

    logger.log(
        LogLevel::Info,
        "Command consumer finished");
}

} // namespace

int main()
{
    LogManager logger(
        "gateway_ack_demo.log");

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
        << "[MAIN] start gateway ACK demo"
        << '\n';

    std::thread producer(
        inputProducer,
        std::ref(logger),
        std::ref(stats));

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
        << "[RESULT] received="
        << stats.received
        << '\n';

    std::cout
        << "[RESULT] accepted="
        << stats.accepted
        << '\n';

    std::cout
        << "[RESULT] rejected="
        << stats.rejected
        << '\n';

    std::cout
        << "[RESULT] responses="
        << stats.ackCount
        << '\n';

    std::cout
        << "[MAIN] all worker threads joined"
        << '\n';

    return 0;
}