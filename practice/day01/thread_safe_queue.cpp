#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue()
        : stopped_(false)
    {
    }

    void push(T value)
    {
       {
        std::lock_guard<std::mutex>lock(mutex_);
        queue_.push(value);
       }
       cv_.notify_one();
    }

    bool wait_and_pop(T &value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock,[this]()
        {
            return !queue_.empty()||stopped_;
        });
        if(stopped_&&queue_.empty())
        {
            return false;
        }
        value=queue_.front();
        queue_.pop();

        return true;
    }

    void stop()
    {
     {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_=true;
     }
     cv_.notify_all();
    }


private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_;
};

int main()
{
    ThreadSafeQueue<int> queue;

    std::thread producer([&queue]()
    {
        for (int i = 1; i <= 5; ++i)
        {
            queue.push(i);

            std::cout
                << "produce: "
                << i
                << '\n';
        }

        queue.stop();
    });

    std::thread consumer([&queue]()
    {
        int value = 0;

        while (queue.wait_and_pop(value))
        {
            std::cout
                << "consume: "
                << value
                << '\n';
        }

        std::cout << "consumer exit\n";
    });

    producer.join();
    consumer.join();

    return 0;
}