# 第五个月 Day17：condition_variable 与生产者消费者模型

## 今日目标

学习 std::condition_variable、std::unique_lock 和 std::queue，将 Day15 的轮询式线程通信升级为等待—通知模型。

## 今日完成内容

1. 学习生产者消费者模型
2. 学习 std::queue
3. 学习 std::condition_variable
4. 学习 std::unique_lock
5. 学习 wait()
6. 学习 notify_one()
7. 学习 notify_all()
8. 理解 predicate
9. 理解虚假唤醒
10. 实现传感器数据线程安全队列
11. 实现生产结束安全通知
12. 实现消费者安全退出
13. 更新 README 和 GitHub

## 生产者消费者模型

生产者负责产生数据：

```text
采集线程
→ 产生 SensorSample
→ push 到 queue
→ notify_one
```

消费者负责处理数据：

```text
处理线程
→ wait
→ 被唤醒
→ pop 数据
→ 处理
```

## std::queue

queue 是 FIFO 队列：

```text
First In First Out
先进先出
```

主要接口：

```cpp
push()
front()
pop()
empty()
size()
```

std::queue 本身不是线程安全容器，因此多线程访问时必须使用 mutex。

## condition_variable

条件变量用于线程之间等待和通知。

消费者没有数据：

```cpp
condition.wait(...)
```

生产者写入数据：

```cpp
condition.notify_one();
```

相比轮询，消费者没有任务时可以真正进入阻塞状态。

## unique_lock

condition_variable::wait() 需要配合：

```cpp
std::unique_lock<std::mutex>
```

因为 wait 需要在内部释放 mutex，并在被唤醒后重新加锁。

lock_guard 不支持这种灵活的主动解锁和重新加锁。

## wait predicate

```cpp
condition.wait(
    lock,
    []
    {
        return !queue.empty() ||
               productionFinished;
    });
```

只有队列存在数据或生产结束时，wait 才真正返回。

predicate 可以处理虚假唤醒。

## 虚假唤醒

线程有可能在没有真正业务通知的情况下从 wait 中醒来，因此消费者不能认为“醒来就一定有数据”。

必须重新检查条件。

## notify_one

```cpp
condition.notify_one();
```

唤醒一个等待线程。

当前只有一个消费者，所以每次生产一条数据后使用 notify_one。

## notify_all

```cpp
condition.notify_all();
```

唤醒所有等待线程。

生产结束时使用 notify_all，方便以后扩展多个消费者。

## 临界区

只在访问以下共享状态时持锁：

```text
sampleQueue
productionFinished
```

数据取出后立即：

```cpp
lock.unlock();
```

后续业务处理不再占用 mutex。

## 为什么不再使用 atomic

productionFinished 与 queue 属于同一组共享状态。

二者统一由 queueMutex 保护，因此使用普通 bool 即可。

## 安全退出

消费者退出条件：

```text
productionFinished == true
并且
sampleQueue.empty() == true
```

这保证最后一条数据也处理完成之后才退出。

## 与 Day15 的区别

Day15：

```text
atomic
+
单槽数据
+
轮询
+
sleep_for
```

Day17：

```text
queue
+
mutex
+
condition_variable
+
wait / notify
```

Day17 更接近正式生产者消费者模型。

## 项目映射

后续 Linux IoT Gateway 可以演化成：

```text
UART 接收线程
→ 消息队列
→ 协议处理线程
→ MQTT / STM32
```

## 今日收获

condition_variable 解决的是“什么时候让线程继续执行”的问题。

mutex 解决的是“谁可以访问共享资源”的问题。

queue 解决的是“多条数据如何排队保存”的问题。

三者组合形成基础生产者消费者模型。

## 明日计划

将生产者消费者模型进一步接入 Linux IoT Gateway 的协议处理流程，并开始整理 C++ 多线程项目面试话术。