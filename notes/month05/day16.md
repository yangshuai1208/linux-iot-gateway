# 第五个月 Day16：mutex 与线程安全 LogManager

## 今日目标

学习数据竞争、临界区、std::mutex 和 std::lock_guard，并为 LogManager 增加线程安全保护。

## 今日完成内容

1. 理解数据竞争
2. 理解临界区
3. 学习 std::mutex
4. 了解 lock 和 unlock
5. 学习 std::lock_guard
6. 理解 RAII 自动解锁
7. 学习 mutable mutex
8. 修改 LogManager 线程安全接口
9. 将容器引用接口改为返回副本
10. 使用两个线程共享写入 LogManager
11. 使用 std::ref 传递引用
12. 验证文件日志、缓存和统计
13. 更新 README 和 GitHub

## 数据竞争

当多个线程并发访问同一内存位置，至少一个线程进行写操作，而且没有正确同步时，会产生数据竞争。

数据竞争会导致未定义行为。

LogManager 中可能被并发访问的资源包括：

- std::ofstream
- std::vector
- std::map
- 配置状态
- std::cout

## 临界区

临界区是同一时间只允许一个线程进入的共享资源访问区域。

LogManager 的一次 log 操作需要完整保护：

```text
终端输出
→ 文件写入
→ 文件刷新
→ 缓存更新
→ 统计更新
```

## std::mutex

mutex 用于保证同一时间只有一个线程进入临界区。

手动使用：

```cpp
mutex.lock();
mutex.unlock();
```

但手动解锁容易因 return 或异常而遗漏。

## std::lock_guard

```cpp
std::lock_guard<std::mutex> lock(mutex);
```

创建时自动加锁，离开作用域时自动解锁。

它通过 RAII 管理锁的生命周期。

## mutable mutex

const 成员函数中也需要保护共享数据。

```cpp
mutable std::mutex mutex_;
```

允许 const 查询函数加锁，但不改变对象的逻辑业务状态。

## 为什么返回副本

返回内部 vector 或 map 的引用时，函数退出后锁已经释放，外部仍可能访问正在被其他线程修改的容器。

因此线程安全接口改为返回副本：

```cpp
std::vector<std::string> getRecentLogs() const;
```

```cpp
std::map<LogLevel, std::size_t>
getAllLogCounts() const;
```

## std::ref

std::thread 默认复制传入参数。

LogManager 包含 ofstream 和 mutex，不能复制。

使用：

```cpp
std::ref(logger)
```

将 logger 以引用形式传给线程函数。

## 实验结果

两个线程分别写入 5 条日志：

```text
collector → 5 条 INFO
processor → 5 条 WARNING
```

结果：

```text
INFO=5
WARNING=5
ERROR=0
cached_logs=10
```

日志顺序不固定，但每条日志内容完整，统计数量正确。

## 生命周期要求

互斥锁不能解决对象已经析构的问题。

所有使用 logger 的线程都必须先 join，然后 logger 才能析构。

## 当前不足

- 最近日志缓存仍使用 vector 头部删除
- 文件写入在锁内，可能增加锁持有时间
- 尚未学习 condition_variable
- 尚未实现日志消息队列
- 尚未测试多个生产者高并发场景

## 明日计划

学习 condition_variable、unique_lock 和 queue，实现不使用轮询的生产者消费者模型。