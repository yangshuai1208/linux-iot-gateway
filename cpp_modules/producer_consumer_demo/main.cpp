#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

namespace
{
    constexpr int kTotalSamples=5;

    struct SensorSample
    {
        int sequence;
        int temperature;
        int humidity;
    };

    std::queue<SensorSample>sampleQueue;

    std::mutex queueMutex;

    std::condition_variable queueCondition;

    bool productionFinished=false;

    void collectTask()
    {
        for(int index=1;index<=kTotalSamples;++index)
        {
            SensorSample sample{index,20+index,50+index};
        
        {
            std::lock_guard<std::mutex>lock(queueMutex);

            sampleQueue.push(sample);
        }
        queueCondition.notify_one();

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    {
        std::lock_guard<std::mutex>lock(queueMutex);

        productionFinished=true;
    }
    queueCondition.notify_all();
    }
    void processTask(int &processCount)
    {
        while(true)
        {
            std::unique_lock<std::mutex>lock(queueMutex);

            queueCondition.wait(lock,[]
                {return !sampleQueue.empty() || productionFinished;});

            if(sampleQueue.empty()&&productionFinished)
            {
                break;
            }
            const SensorSample sample=sampleQueue.front();

            sampleQueue.pop();

            lock.unlock();

            ++processCount;

            std::cout
            <<"[PROCESS]"
            <<"sequence="
            <<sample.sequence
            <<"temperature="
            <<sample.temperature
            <<"humidity="
            <<sample.humidity
            <<"processed_count="
            <<processCount
            <<'\n';
        }
    }
}
int main()
{
    int processedCount=0;

    std::cout<<"[MAIN] start producer_consumer demo"
    <<'\n';

    std::thread collector(collectTask);

    std::thread processor(processTask,std::ref(processedCount));

    if(collector.joinable())
    {
        collector.join();
    }
    if(processor.joinable())
    {
        processor.join();
    }
    std::cout
    <<"[RESULT] processed_count="
    <<processedCount
    <<'\n';

    std::cout
    <<"[MAIN] all worker threads joined"
    <<'\n';

    return 0;
}