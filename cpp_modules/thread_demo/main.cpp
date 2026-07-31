#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

namespace
{
    constexpr int kTotalSamples=5;

    std::atomic<int>latestSample{0};

    std::atomic<bool>collectionFinished{false};

    std::atomic<bool> sampleReady{false};
    void collectTask()
    {
        for(int index=1;index<=kTotalSamples;++index)
        {
            while(sampleReady.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

        const int sample=20+index;

        latestSample.store(sample);
        sampleReady.store(true);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        collectionFinished.store(true);
    }
    void processTask()
    {
        int processedCount=0;

        while(!collectionFinished.load()||sampleReady.load())
        {
            if(sampleReady.exchange(false))
            {
                const int sample=latestSample.load();

                ++processedCount;

                std::cout
                <<"[PROCESS]thread="
                <<std::this_thread::get_id()
                <<"sample="
                <<sample
                <<"processed_count="
                <<processedCount
                <<'\n';
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
        std::cout
        <<"[PROCESS]finished,total="
        <<processedCount
        <<'\n';
    }
}
int main()
{
    std::cout
    <<"[MAIN] thread="
    <<std::this_thread::get_id()
    <<'\n';

    std::thread collector(collectTask);
    std::thread processor(processTask);

    std::cout
    <<"[MIAN] collector thread="
    <<collector.get_id()
    <<'\n';

    std::cout
    <<"[MIAN] processor thread="
    <<processor.get_id()
    <<'\n';

    if(collector.joinable())
    {
        collector.join();
    }
    if(processor.joinable())
    {
        processor.join();
    }
    std::cout
    <<"[MAIN] all worker threads joined"
    <<'\n';

    return 0;
}

