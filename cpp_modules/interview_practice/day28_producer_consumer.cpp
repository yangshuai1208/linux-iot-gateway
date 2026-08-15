#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

std::queue<std::string> commandQueue;

std::mutex queueMutex;

std::condition_variable queueCondition;

bool productionFinished = false;

void commandProducer()
{
    const std::vector<std::string> commands{
        "OPEN",
        "GRAB",
        "RELEASE",
        "STOP",
        "OPEN"};

    for (const auto &command : commands)
    {
        {
            std::lock_guard<std::mutex> lock(
                queueMutex);

            commandQueue.push(command);

            std::cout
                << "[PRODUCER] push: "
                << command
                << '\n';
        }

        queueCondition.notify_one();

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100));
    }

    {
        std::lock_guard<std::mutex> lock(
            queueMutex);

        productionFinished = true;
    }

    queueCondition.notify_all();

    std::cout
        << "[PRODUCER] finished"
        << '\n';
}

void commandConsumer()
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
                       productionFinished;
            });

        if (commandQueue.empty() &&
            productionFinished)
        {
            break;
        }

        const std::string command =
            commandQueue.front();

        commandQueue.pop();

        lock.unlock();

        std::cout
            << "[CONSUMER] process: "
            << command
            << '\n';

        std::this_thread::sleep_for(
            std::chrono::milliseconds(50));
    }

    std::cout
        << "[CONSUMER] exit"
        << '\n';
}

int main()
{
    std::thread producerThread(
        commandProducer);

    std::thread consumerThread(
        commandConsumer);

    producerThread.join();

    consumerThread.join();

    std::cout
        << "[MAIN] all threads finished"
        << '\n';

    return 0;
}