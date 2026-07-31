# 第五个月 Day15：std::thread 多线程基础

## 今日目标

学习 C++ std::thread、join、joinable、线程 ID 和 atomic，并完成采集线程与处理线程双线程 Demo。

## 今日完成内容

1. 理解进程与线程
2. 学习 std::thread
3. 创建采集线程
4. 创建处理线程
5. 学习线程函数
6. 获取线程 ID
7. 学习 join()
8. 学习 joinable()
9. 了解 detach()
10. 学习 sleep_for()
11. 学习 std::atomic
12. 实现线程安全退出
13. 使用 Makefile 和 -pthread 编译
14. 更新 README 和 GitHub

## 进程与线程

进程是正在运行的程序及其资源集合。

线程是进程中的执行流。

同一个进程中的线程通常共享：

- 代码区
- 全局变量
- 堆内存
- 文件描述符
- 进程资源

每个线程通常拥有自己的：

- 栈
- 寄存器上下文
- 程序计数器
- 线程 ID

因为线程共享进程资源，所以通信方便，但同时可能产生数据竞争。

## std::thread

创建线程：

```cpp
std::thread collector(collectTask);
```

collectTask 是线程函数，collector 是管理线程的对象。

线程创建后可能立即执行，不能假设固定执行顺序。

## join()

```cpp
collector.join();
```

join 会阻塞当前线程，直到目标线程执行结束。

本项目中主线程分别等待采集线程和处理线程结束，确保所有工作完成后程序才退出。

## joinable()

```cpp
if (collector.joinable())
{
    collector.join();
}
```

joinable 用于判断 thread 对象当前是否关联一个可以 join 的线程。

线程只能成功 join 一次。

## detach()

detach 会让线程脱离 thread 对象独立运行。

分离后不能再 join，也不方便控制线程结束。

嵌入式网关中的采集、处理和通信线程通常需要明确启动与停止，所以当前不使用 detach。

## 线程对象析构风险

如果 std::thread 对象仍然 joinable，却直接进入析构函数，程序会调用 std::terminate。

因此线程对象销毁前必须：

```text
join()
或
detach()
```

本项目统一使用 join。

## 线程 ID

获取当前线程 ID：

```cpp
std::this_thread::get_id();
```

获取 thread 对象管理的线程 ID：

```cpp
collector.get_id();
```

线程 ID 适合用于调试和区分日志来源。

## sleep_for()

```cpp
std::this_thread::sleep_for(
    std::chrono::milliseconds(200));
```

使当前线程休眠指定时间。

本项目用它模拟传感器采集周期，也用于降低空轮询时的 CPU 占用。

## std::atomic

使用：

```cpp
std::atomic<int> latestSample;
std::atomic<bool> sampleReady;
```

atomic 可以保证简单读写操作的原子性，避免多个线程同时访问普通变量产生数据竞争。

常用操作：

```cpp
load();
store();
exchange();
```

## exchange()

```cpp
if (sampleReady.exchange(false))
{
}
```

exchange 会返回修改前的值，并将变量更新为新值。

本项目中用于原子地获取“是否有新数据”并将状态恢复为空。

## 当前数据流

```text
采集线程
→ latestSample.store()
→ sampleReady.store(true)

处理线程
→ sampleReady.exchange(false)
→ latestSample.load()
→ 处理数据
```

采集结束后：

```cpp
collectionFinished.store(true);
```

处理线程需要同时判断：

```text
采集是否结束
以及
是否还有最后一条数据未处理
```

## 当前方案的限制

本项目使用单槽共享区和轮询，适合基础演示，但不适合正式网关。

存在：

- 忙等待
- 不能缓存多条数据
- 扩展复杂数据困难
- 实时性和效率有限
- 只适合简单状态通信

后续将使用：

```text
std::mutex
std::lock_guard
std::condition_variable
std::queue
```

实现更完整的线程通信。

## LogManager 暂未接入多线程

当前 LogManager 中包含：

- ofstream
- vector
- map
- 状态变量

这些成员还没有 mutex 保护。

多个线程同时调用 log() 可能产生数据竞争，因此 Day15 暂时不让多个线程共享调用 LogManager。

## 实验结果

采集线程生成 5 条模拟数据：

```text
21
22
23
24
25
```

处理线程完整处理 5 条数据。

主线程通过 join 等待两个工作线程结束后安全退出。

## 今日收获

std::thread 用于创建线程，join 用于等待线程结束。

线程之间共享数据时必须考虑同步问题，不能把单线程对象直接放到多个线程中使用。

## 明日计划

学习 std::mutex、lock_guard 和数据竞争，为 LogManager 增加互斥锁，完成线程安全日志输出 Demo。